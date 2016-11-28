#include "remotenode.h"
#include "convert.h"

RemoteNode::RemoteNode(alpha::protort::parser::node const& node_information)
    : node_information_(node_information)
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();
}

void RemoteNode::init(boost::asio::io_service &service)
{
    client_ = boost::make_shared<client_t>(this->shared_from_this(), service);

    boost::asio::ip::tcp::endpoint ep(
                boost::asio::ip::address::from_string(node_information_.address),
                100);

    client_->async_connect(ep);
}

void RemoteNode::shutdown()
{
    client_->stop_trying_to_connect();

    client_.reset();
}

void RemoteNode::async_deploy(deploy_configuration& deploy_configuration_)
{
    std::string current_node = node_information_.name;

    std::set<std::string> added_nodes_;
    std::set<std::string> added_maps_;

    alpha::protort::protocol::Packet_Payload payload_;

    payload_.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::DeployConfig);
    alpha::protort::protocol::deploy::Config* configuration_ =
        payload_.mutable_deploy_packet()->mutable_request()->mutable_deploy_config()->mutable_config();

    configuration_->mutable_this_node_info()->set_name(current_node);
    configuration_->mutable_this_node_info()->set_port(deploy_configuration_.map_node[current_node].port);

    alpha::protort::protocol::deploy::NodeInfo* node_info_ = configuration_->add_node_infos();
    node_info_->set_name(current_node);
    node_info_->set_port(deploy_configuration_.map_node[current_node].port);
    node_info_->set_address(deploy_configuration_.map_node[current_node].address);

    for (auto &component : deploy_configuration_.map_node_with_components[current_node])
    {
        alpha::protort::protocol::ComponentKind kind_ =
            alpha::protort::components::get_component_kind(
                deploy_configuration_.map_components[component.comp_name].kind);

        // Добавляем компонент к конфигурации нода
        alpha::protort::protocol::deploy::Instance* instance_ = configuration_->add_instances();

        instance_->set_name(component.comp_name);
        instance_->set_kind(kind_);

        // Указываем, что компонент относится к текущему ноду
        alpha::protort::protocol::deploy::Map* components_map_ = configuration_->add_maps();

        components_map_->set_node_name(current_node);
        components_map_->set_instance_name(component.comp_name);

        for (auto &connection : deploy_configuration_.map_component_with_connections[component.comp_name])
        {
            // Добавляем коннекшион к конфигурации нода
            alpha::protort::protocol::deploy::Connection* connection_ = configuration_->add_connections();

            connection_->mutable_source()->set_name(connection.source);
            connection_->mutable_source()->set_port(connection.source_out);

            connection_->mutable_destination()->set_name(connection.dest);
            connection_->mutable_destination()->set_port(connection.dest_in);

            // Получаем имя нода компонента назначения
            std::string node_name_ = deploy_configuration_.map_component_node[connection.dest].node_name;

            if(node_name_ != current_node)
            {
                alpha::protort::parser::node& node_ = deploy_configuration_.map_node[node_name_];

                if(added_nodes_.find(node_.name) == added_nodes_.end())
                {
                    // Добавляем информацию о ноде в конфигурацию
                    alpha::protort::protocol::deploy::NodeInfo* remote_node_info_ = configuration_->add_node_infos();

                    remote_node_info_->set_name(node_.name);
                    remote_node_info_->set_port(node_.port);
                    remote_node_info_->set_address(node_.address);
                    added_nodes_.insert(node_.name);
                }

                if(added_maps_.find(connection.dest) == added_maps_.end())
                {
                    // Добавляем информацию о мэпинге удаленного компонента
                    alpha::protort::protocol::deploy::Map* components_map_ = configuration_->add_maps();

                    components_map_->set_node_name(node_.name);
                    components_map_->set_instance_name(connection.dest);
                    added_maps_.insert(connection.dest);
                }
            }
        }
    }

    boost::shared_ptr<alpha::protort::protolink::request_callbacks> ptr_(
        new alpha::protort::protolink::request_callbacks);

    ptr_->on_finished.connect
        (
            [&](const alpha::protort::protocol::Packet_Payload& p)
              {
                  emit deployConfigRequestFinished();
              }
        );

    client_->async_send_request(payload_, ptr_);
}

void RemoteNode::async_start(alpha::protort::protocol::Packet_Payload &packet)
{
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> ptr_(
        new alpha::protort::protolink::request_callbacks);

    ptr_->on_finished.connect
        (
            [&](const alpha::protort::protocol::Packet_Payload& p)
              {
                  emit statusRequestFinished(p.deploy_packet());
              }
        );
    client_->async_send_request(packet, ptr_);
}

void RemoteNode::async_stop(alpha::protort::protocol::Packet_Payload &packet)
{
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> ptr_(
        new alpha::protort::protolink::request_callbacks);

    ptr_->on_finished.connect
        (
            [&](const alpha::protort::protocol::Packet_Payload& p)
              {
                  emit stopRequestFinished();
              }
        );
    client_->async_send_request(packet, ptr_);
}

void RemoteNode::async_status(alpha::protort::protocol::Packet_Payload& status)
{
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> ptr_(
        new alpha::protort::protolink::request_callbacks);

    ptr_->on_finished.connect
        (
            [&](const alpha::protort::protocol::Packet_Payload& p)
              {
                  emit startRequestFinished();
              }
        );
    client_->async_send_request(status, ptr_);
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
