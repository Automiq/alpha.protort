#include "remotenode.h"
#include "convert.h"
#include "remotecomponent.h"

#include "deploy.pb.h"

RemoteNode::RemoteNode(const parser::node &node_information,
                       protocol::backup::BackupStatus backup_status):
                       node_information_(node_information),
                       backup_status_(backup_status),
                       uptime_(0),
                       packetsReceived_(0),
                       bytesReceived_(0),
                       packetsSent_(0),
                       bytesSent_(0),
                       downSpeed_(0),
                       upSpeed_(0),
                       isConnected_(false)
{
    qRegisterMetaType<protocol::Packet_Payload>();
    qRegisterMetaType<protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();

    connect(this, &RemoteNode::connected, this, &RemoteNode::onConnected);
    connect(this, &RemoteNode::connectionFailed, this, &RemoteNode::onConnectionFailed);
    connect(this, &RemoteNode::statusRequestFinished, this, &RemoteNode::onStatusRequestFinished);

    name_ = QString::fromStdString(node_information_.name);
}

RemoteNode::~RemoteNode()
{
    qDeleteAll(components_);
}

void RemoteNode::init(boost::asio::io_service &service)
{

    boost::asio::ip::tcp::endpoint ep(
                boost::asio::ip::address::from_string(node_information_.host.ip_address),
                node_information_.host.config_port);

    client_ = boost::make_shared<client_t>(this->shared_from_this(), service , ep);

    client_->async_connect(ep);
}

void RemoteNode::init_pairnode(boost::shared_ptr<RemoteNode> &pairnode)
{
   this->pairnode_ = boost::weak_ptr<RemoteNode>(pairnode);
}

void RemoteNode::shutdown()
{
    client_->prepare_shutdown();
    client_.reset();
}

QString RemoteNode::name() const
{
    return QString::fromStdString(node_information_.name);
}

QString RemoteNode::address() const
{
    return QString::fromStdString(node_information_.host.ip_address);
}

QString RemoteNode::info() const
{
    return QString("%1 (%2)").arg(name()).arg(address());
}

boost::weak_ptr<RemoteNode> RemoteNode::pairnode() const
{
    return this->pairnode_;
}

void RemoteNode::init_info_node(const std::string &name_node,
                                const parser::address &address,
                                protocol::backup::BackupStatus backup_status,
                                protocol::deploy::NodeInfo &node_info) const
{
    node_info.set_name(name_node);
    node_info.set_address(address.ip_address);
    node_info.set_port(address.port);
    node_info.set_backup_status(backup_status);
}

void RemoteNode::init_backup_status_node_infos(const RemoteNode &pairnode,
                                               protocol::deploy::Config &configuration)
{
    protocol::deploy::NodeInfo *node_info = configuration.add_node_infos();

    init_info_node(pairnode.node_information_.name,
                   pairnode.node_information_.host,
                   pairnode.backup_status_,
                   *node_info);

    if(pairnode.backup_status_ != protocol::backup::BackupStatus::None){
        if(auto tmp_pairnode = pairnode.pairnode_.lock()){
            node_info = configuration.add_node_infos();

            init_info_node(node_information_.name,
                           tmp_pairnode->node_information_.host,
                           tmp_pairnode->backup_status_,
                           *node_info);
        }
    }
}

RemoteNode &RemoteNode::search_pairnode(const std::string &name_node,
                                         const QList<RemoteNodePtr> &remote_nodes) const
{
    auto result = std::find_if(remote_nodes.begin(), remote_nodes.end(), [name_node](const RemoteNodePtr remote_node){return remote_node->node_information_.name == name_node;});

    if(result == remote_nodes.end()){
            // rugaemsa
    }

    return *(*result);
}

void RemoteNode::async_deploy(deploy_configuration &deploy_configuration, const QList<RemoteNodePtr> &remote_node)
{
    std::string current_node = node_information_.name;

    std::set<std::string> added_nodes;
    std::set<std::string> added_maps;

    protocol::Packet_Payload payload;

    payload.mutable_deploy_packet()->set_kind(protocol::deploy::DeployConfig);

    protocol::deploy::Config *configuration =
        payload.mutable_deploy_packet()->mutable_request()->mutable_deploy_config()->mutable_config();

    init_info_node(current_node,
                   node_information_.host,
                   backup_status_,
                   *configuration->mutable_this_node_info());

    init_backup_status_node_infos(*this, *configuration);

    for (auto &component : deploy_configuration.map_node_with_components[current_node]){
        protocol::ComponentKind kind =
                components::get_component_kind(deploy_configuration.map_components[component.comp_name].kind);

        // Добавляем компонент к конфигурации нода
        protocol::deploy::Instance *instance = configuration->add_instances();

        instance->set_name(component.comp_name);
        instance->set_kind(kind);

        // Указываем, что компонент относится к текущему ноду
        protocol::deploy::Map *components_map_ = configuration->add_maps();

        components_map_->set_node_name(current_node);
        components_map_->set_instance_name(component.comp_name);

        for (auto &connection : deploy_configuration.map_component_with_connections[component.comp_name]){
            // Добавляем коннекшион к конфигурации нода
            protocol::deploy::Connection *connection_ = configuration->add_connections();

            connection_->mutable_source()->set_name(connection.source);
            connection_->mutable_source()->set_port(connection.source_out);

            connection_->mutable_destination()->set_name(connection.dest);
            connection_->mutable_destination()->set_port(connection.dest_in);

            // Получаем имя нода компонента назначения
            std::string node_name = deploy_configuration.map_component_node[connection.dest].node_name;

            if(node_name != current_node){
                RemoteNode &node_ = search_pairnode(node_name, remote_node);

                if(added_nodes.find(node_.node_information_.name) == added_nodes.end()){
                    // Добавляем информацию о ноде в конфигурацию
                    init_backup_status_node_infos(node_, *configuration);

                    added_nodes.insert(node_.node_information_.name);
                }

                if(added_maps.find(connection.dest) == added_maps.end()){
                    // Добавляем информацию о мэпинге удаленного компонента
                    protocol::deploy::Map *components_map_ = configuration->add_maps();

                    components_map_->set_node_name(node_.node_information_.name);
                    components_map_->set_instance_name(connection.dest);

                    added_maps.insert(connection.dest);
                }
            }
        }
    }

    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const protocol::Packet_Payload& packet) {
        emit deployConfigRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(payload, callbacks);
}

void RemoteNode::async_start(protocol::Packet_Payload &packet)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const protocol::Packet_Payload& packet) {
        emit startRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(packet, callbacks);
}

void RemoteNode::async_stop(protocol::Packet_Payload &packet)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const protocol::Packet_Payload& packet) {
        emit stopRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(packet, callbacks);
}

void RemoteNode::async_status(protocol::Packet_Payload &status)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const protocol::Packet_Payload& packet) {
        emit statusRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(status, callbacks);
}

void RemoteNode::on_connected(const boost::system::error_code &err)
{
    if (!err){
        emit connected();
        return;
    }
    emit connectionFailed(err);
}

void RemoteNode::on_packet_sent(const boost::system::error_code &err, size_t bytes)
{

}

void RemoteNode::on_new_packet(protocol::Packet_Payload packet)
{

}

void RemoteNode::appendComponent(RemoteComponent *component)
{
    components_.push_back(component);
    component->setParent(this);
}

QList<RemoteComponent *> RemoteNode::components() const
{
    return components_;
}

//! Методы изменения данных узла
void RemoteNode::setUptime(uint32_t time)
{
    uptime_  = time;
}

void RemoteNode::setPacketsReceived(uint32_t value)
{
    packetsReceived_ = value;
}

void RemoteNode::setBytesReceived(uint32_t value)
{
    bytesReceived_ = value;
}

void RemoteNode::setPacketsSent(uint32_t value)
{
    packetsSent_ = value;
}

void RemoteNode::setBytesSent(uint32_t value)
{
    bytesSent_ = value;
}

void RemoteNode::setConnected(bool value)
{
    isConnected_ = value;

    emit statusChanged();
}

double RemoteNode::calcUpSpeed(const QTime &now, uint32_t bytesSent)
{
    return calcSpeed(now, bytesSent_, bytesSent);
}

double RemoteNode::calcDownSpeed(const QTime &now, uint32_t bytesReceived)
{
    return calcSpeed(now, bytesReceived_, bytesReceived);
}

double RemoteNode::calcSpeed(const QTime &now, uint32_t lastBytes, uint32_t nowBytes)
{
    if (m_lastStatusTime.isNull()){
        return 0;
    }

    auto msecs = m_lastStatusTime.msecsTo(now);

    return (double(nowBytes - lastBytes) / msecs) * 1000;
}

//! Методы получения данных узла
bool RemoteNode::isConnected() const
{
    return isConnected_;
}

uint32_t RemoteNode::uptime() const
{
    return uptime_;
}

uint32_t RemoteNode::packetsReceived() const
{
    return packetsReceived_;
}

uint32_t RemoteNode::packetsSent() const
{
    return packetsSent_;
}

uint32_t RemoteNode::bytesReceived() const
{
    return bytesReceived_;
}

uint32_t RemoteNode::bytesSent() const
{
    return bytesSent_;
}

uint32_t RemoteNode::downSpeed() const
{
    return downSpeed_;
}

uint32_t RemoteNode::upSpeed() const
{
    return upSpeed_;
}

RemoteComponent *RemoteNode::componentAt(int index) const
{
    return components_[index];
}

void RemoteNode::onStatusRequestFinished(const protocol::deploy::Packet &packet)
{
    auto status = packet.response().status();

    auto cSize = status.component_statuses_size();

    if (cSize > 0 && components_.size() == 0){
        for (auto i = 0, size = status.component_statuses_size(); i < size; ++i){
            appendComponent(new RemoteComponent);
        }

        emit componentsChanged();
    }

    for (auto i = 0, size = status.component_statuses_size(); i < size; ++i){
        auto cs = status.component_statuses(i);

        auto c = components_[i];

        c->setName(QString::fromStdString(cs.name()));
        c->setPacketsReceived(cs.in_packet_count());
        c->setPacketsSent(cs.out_packet_count());
    }

    auto now = QTime::currentTime();

    downSpeed_ = calcDownSpeed(now, status.in_bytes_count());
    upSpeed_ = calcUpSpeed(now, status.out_bytes_count());

    m_lastStatusTime = now;

    setUptime(status.uptime());
    setPacketsReceived(status.in_packets_count());
    setPacketsSent(status.out_packets_count());
    setBytesReceived(status.in_bytes_count());
    setBytesSent(status.out_bytes_count());

    emit statusChanged();
}

void RemoteNode::onConnected()
{
    setConnected(true);
}

void RemoteNode::onConnectionFailed(const boost::system::error_code &)
{
    setConnected(false);
}
