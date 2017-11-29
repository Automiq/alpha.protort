#include "remotenode.h"
#include "convert.h"
#include "remotecomponent.h"

#include "deploy.pb.h"

RemoteNode::RemoteNode(alpha::protort::parser::node const& node_information)
    : node_information_(node_information),
      uptime_(0),
      packetsReceived_(0),
      bytesReceived_(0),
      packetsSent_(0),
      bytesSent_(0),
      downSpeed_(0),
      upSpeed_(0),
      isConnected_(false)
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
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
    client_ = boost::make_shared<client_t>(this->shared_from_this(), service);

    boost::asio::ip::tcp::endpoint ep(
                boost::asio::ip::address::from_string(node_information_.host.ip_address),
                node_information_.host.config_port);

    client_->async_connect(ep);
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

void RemoteNode::init_info_node(const alpha::protort::parser::node &node,
                                const std::string &name_node,
                                alpha::protort::protocol::deploy::NodeInfo* node_info)
{
    node_info->set_name(name_node);
    node_info->set_address(node.host.ip_address);
    node_info->set_port(node.host.port);
}

void RemoteNode::init_backup_status_node_info(const alpha::protort::parser::node &node,
                                              const std::string &name_node,
                                              alpha::protort::protocol::deploy::Config* configuration,
                                              alpha::protort::protocol::deploy::NodeInfo* node_info)
{
    if(!node_info)
       node_info = configuration->add_node_infos();

    init_info_node(node, name_node, node_info);

    if(node.pairnode){
        node_info->set_backup_status(alpha::protort::protocol::backup::BackupStatus::Master);
    }
    else if(node.name.find("pairnode.") != -1){
        node_info->set_backup_status(alpha::protort::protocol::backup::BackupStatus::Slave);
    }
    else{
        node_info->set_backup_status(alpha::protort::protocol::backup::BackupStatus::None);
    }
}

void RemoteNode::async_deploy(deploy_configuration& deploy_configuration)
{
    std::string current_node = node_information_.name;

    std::set<std::string> added_nodes;
    std::set<std::string> added_maps;

    alpha::protort::protocol::Packet_Payload payload;

    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::DeployConfig);

    alpha::protort::protocol::deploy::Config* configuration =
        payload.mutable_deploy_packet()->mutable_request()->mutable_deploy_config()->mutable_config();

    init_backup_status_node_info(node_information_, node_information_.name, configuration, configuration->mutable_this_node_info());
    init_backup_status_node_info(node_information_, node_information_.name, configuration);

    if(node_information_.pairnode){
        std::string pairnode_name = (boost::format("pairnode.%1%") % node_information_.name).str();
        alpha::protort::parser::node slave_node = deploy_configuration.map_node[pairnode_name];

        init_backup_status_node_info(slave_node, node_information_.name, configuration);
    }

    if(current_node.find("pairnode.") != -1){
        std::string node_name = current_node.substr(9);
        alpha::protort::parser::node master_node = deploy_configuration.map_node[node_name];

         init_backup_status_node_info(master_node, current_node, configuration);
    }

    for (auto &component : deploy_configuration.map_node_with_components[current_node])
    {
        alpha::protort::protocol::ComponentKind kind =
            alpha::protort::components::get_component_kind(
                deploy_configuration.map_components[component.comp_name].kind);

        // Добавляем компонент к конфигурации нода
        alpha::protort::protocol::deploy::Instance* instance = configuration->add_instances();

        instance->set_name(component.comp_name);
        instance->set_kind(kind);

        // Указываем, что компонент относится к текущему ноду
        alpha::protort::protocol::deploy::Map* components_map_ = configuration->add_maps();

        components_map_->set_node_name(current_node);
        components_map_->set_instance_name(component.comp_name);

        for (auto &connection : deploy_configuration.map_component_with_connections[component.comp_name])
        {
            // Добавляем коннекшион к конфигурации нода
            alpha::protort::protocol::deploy::Connection* connection_ = configuration->add_connections();

            connection_->mutable_source()->set_name(connection.source);
            connection_->mutable_source()->set_port(connection.source_out);

            connection_->mutable_destination()->set_name(connection.dest);
            connection_->mutable_destination()->set_port(connection.dest_in);

            // Получаем имя нода компонента назначения
            std::string node_name = deploy_configuration.map_component_node[connection.dest].node_name;

            if(node_name != current_node)
            {
                alpha::protort::parser::node& node_ = deploy_configuration.map_node[node_name];

                if(added_nodes.find(node_.name) == added_nodes.end())
                {
                    // Добавляем информацию о ноде в конфигурацию
                    init_backup_status_node_info(node_, node_.name, configuration);

                    added_nodes.insert(node_.name);
                }

                if(added_maps.find(connection.dest) == added_maps.end())
                {
                    // Добавляем информацию о мэпинге удаленного компонента
                    alpha::protort::protocol::deploy::Map* components_map_ = configuration->add_maps();

                    components_map_->set_node_name(node_.name);
                    components_map_->set_instance_name(connection.dest);
                    added_maps.insert(connection.dest);
                }
            }
        }
    }

    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet) {
        emit deployConfigRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(payload, callbacks);
}

void RemoteNode::async_start(alpha::protort::protocol::Packet_Payload &packet)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet) {
        emit startRequestFinished(packet.deploy_packet());
    });
    client_->async_send_request(packet, callbacks);
}

void RemoteNode::async_stop(alpha::protort::protocol::Packet_Payload &packet)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet) {
        emit stopRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(packet, callbacks);
}

void RemoteNode::async_status(alpha::protort::protocol::Packet_Payload& status)
{
    auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

    callbacks->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet) {
        emit statusRequestFinished(packet.deploy_packet());
    });

    client_->async_send_request(status, callbacks);
}

void RemoteNode::on_connected(const boost::system::error_code& err)
{
    if (!err)
    {
        emit connected();
        return;
    }
    emit connectionFailed(err);
}

void RemoteNode::on_packet_sent(const boost::system::error_code& err, size_t bytes)
{

}

void RemoteNode::on_new_packet(alpha::protort::protocol::Packet_Payload packet)
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
    if (m_lastStatusTime.isNull())
        return 0;

    auto msecs = m_lastStatusTime.msecsTo(now);
    return (double(nowBytes - lastBytes) / msecs) * 1000;
}

//! Методы получения данных узла
bool RemoteNode::isConnected() const { return isConnected_; }
uint32_t RemoteNode::uptime() const { return uptime_; }

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

void RemoteNode::onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet &packet)
{
    auto status = packet.response().status();

    auto cSize = status.component_statuses_size();

    if (cSize > 0 && components_.size() == 0)
    {
        for (auto i = 0, size = status.component_statuses_size(); i < size; ++i)
            appendComponent(new RemoteComponent);
        emit componentsChanged();
    }

    for (auto i = 0, size = status.component_statuses_size(); i < size; ++i)
    {
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
