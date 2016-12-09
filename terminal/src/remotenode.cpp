#include "remotenode.h"
#include "convert.h"
#include "remotecomponent.h"

#include "deploy.pb.h"

RemoteNode::RemoteNode(alpha::protort::parser::node const& node_information)
    : node_information_(node_information)
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();

    connect(this, &RemoteNode::statusRequestFinished, this, &RemoteNode::onStatusRequestFinished);

    name_ = QString::fromStdString(node_information_.name);
}

RemoteNode::~RemoteNode()
{
    int size = components_.size();
    for(int i = 0; i < size; ++i)
        delete components_[i];
}

void RemoteNode::init(boost::asio::io_service &service)
{
    client_ = boost::make_shared<client_t>(this->shared_from_this(), service);

    boost::asio::ip::tcp::endpoint ep(
                boost::asio::ip::address::from_string(node_information_.address),
                node_information_.config_port);

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
    return QString::fromStdString(node_information_.address);
}

QString RemoteNode::info() const
{
    return QString("%1 (%2)").arg(name()).arg(address());
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

    configuration->mutable_this_node_info()->set_name(current_node);
    configuration->mutable_this_node_info()->set_port(deploy_configuration.map_node[current_node].port);

    alpha::protort::protocol::deploy::NodeInfo* node_info_ = configuration->add_node_infos();
    node_info_->set_name(current_node);
    node_info_->set_port(deploy_configuration.map_node[current_node].port);
    node_info_->set_address(deploy_configuration.map_node[current_node].address);

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
                    alpha::protort::protocol::deploy::NodeInfo* remote_node_info_ = configuration->add_node_infos();

                    remote_node_info_->set_name(node_.name);
                    remote_node_info_->set_port(node_.port);
                    remote_node_info_->set_address(node_.address);
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

QList<RemoteComponent *> RemoteNode::components() const
{
    return components_;
}

//! Методы изменения данных узла
void RemoteNode::setName(QString name){ name_ = name; }
void RemoteNode::setConnect(uint32_t con){ connection_ = con; }
void RemoteNode::setUptime(uint32_t time){ uptime_  = time; }
void RemoteNode::setSpeed(uint32_t speed){ speed_ = speed; }
void RemoteNode::setOutput(uint32_t packets, uint32_t bytes){ in_(packets, bytes); }
void RemoteNode::setInput(uint32_t packets, uint32_t bytes){ out_(packets, bytes); }

//! Методы получения данных узла
QString RemoteNode::name() { return name_; }
bool RemoteNode::isConnect() const { return connection_; }
uint32_t RemoteNode::uptime() const { return uptime_; }
uint32_t RemoteNode::speed() const { return speed_; }
RemoteNode::Packet RemoteNode::output() const { return in_; }
RemoteNode::Packet RemoteNode::input() const { return out_; }

void RemoteNode::onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet &packet)
{
    auto status = packet.response().status();

    setName(QString::fromStdString(status.node_name()));
    setUptime(status.uptime());
    setInput(status.in_packets_count(), status.in_bytes_count());
    setOutput(status.out_packets_count(), status.out_bytes_count());
    setConnect(1); //?????

    RemoteComponent *comp;

    for (auto i = 0, size = status.component_statuses_size(); i < size; ++i)
    {
        auto component = status.component_statuses(i);

        comp = new RemoteComponent;

        comp->setName(QString::fromStdString(component.name()));
        comp->setInput(component.in_packet_count());
        comp->setOutput(status.component_statuses(i).out_packet_count());
        components_.push_back(comp);
        emit componentsChanged();
    }
 emit statusChanged();
//    auto cSize = p.components().size()
//    if (cSize > 0 && components_.size() == 0)
//    {
//            //p.name_;
//            //p.name(); - имя пакета
//        //auto comps = components();
//        // TODO: сконструировать компоненты

//        emit componentsChanged();
//    }

    // TODO: заполнить данные по узлу и его компонентам


}

RemoteNode::Packet RemoteNode::Packet::operator()(uint32_t p, uint32_t b)
{ return RemoteNode::Packet(p, b); }

void RemoteNode::Packet::clear(){packets_ = 0; bytes_ = 0;}

void RemoteNode::addComp(RemoteComponent *comp)
{
    components_.push_back(comp);
}
