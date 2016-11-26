#ifndef TERMINAL_CLIENT_H
#define TERMINAL_CLIENT_H

#include <iostream>
#include <QMetaType>
#include <set>

#include "client.h"
#include "parser.h"

namespace alpha {
namespace protort {
namespace protolink {

Q_DECLARE_METATYPE(alpha::protort::protocol::Packet_Payload);
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::StatusResponse);

template<class Terminal>
class terminal_client:
        public boost::enable_shared_from_this<terminal_client<Terminal>>,
        public QObject
{
public:

    terminal_client(Terminal& terminal, boost::asio::io_service& service, QString node_name)
        : client_(*this, service),
          terminal_(terminal),
          node_name_(node_name)
    {
        qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
        qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    }

    client<terminal_client> client_;

    void start_node(const alpha::protort::protocol::Packet_Payload& p)
    {
         QMetaObject::invokeMethod(
             this->shared_from_this(),
             "on_start_finished",
             Qt::QueuedConnection,
             Q_ARG(alpha::protort::protocol::Packet_Payload, p));
    }

    void stop_node(const alpha::protort::protocol::Packet_Payload& p)
    {
         QMetaObject::invokeMethod(
             this->shared_from_this(),
             "on_stop_finished",
             Qt::QueuedConnection,
             Q_ARG(alpha::protort::protocol::Packet_Payload, p));
    }

    void status_node(const alpha::protort::protocol::Packet_Payload& p)
    {
        alpha::protort::protocol::deploy::StatusResponse status_ =
           p.deploy_packet().response().status();

        QMetaObject::invokeMethod(
            this->shared_from_this(),
            "on_status_response",
            Qt::QueuedConnection,
            Q_ARG(alpha::protort::protocol::deploy::StatusResponse, status_));
    }

    void on_connected(const boost::system::error_code& err)
    {
        if (!err)
        {
            std::string current_node =  node_name_.toStdString();

            std::set<std::string> added_nodes_;
            std::set<std::string> added_maps_;

            alpha::protort::protocol::Packet_Payload payload_;

            payload_.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::DeployConfig);
            alpha::protort::protocol::deploy::Config* configuration_ =
                payload_.mutable_deploy_packet()->mutable_request()->mutable_deploy_config()->mutable_config();

            configuration_->mutable_this_node_info()->set_name(current_node);
            configuration_->mutable_this_node_info()->set_port(terminal_.deploy_config_.map_node[current_node].port);

            alpha::protort::protocol::deploy::NodeInfo* node_info_ = configuration_->add_node_infos();
            node_info_->set_name(current_node);
            node_info_->set_port(terminal_.deploy_config_.map_node[current_node].port);
            node_info_->set_address(terminal_.deploy_config_.map_node[current_node].address);

            for (auto &component : terminal_.deploy_config_.map_node_with_components[current_node])
            {
                alpha::protort::protocol::ComponentKind kind_ =
                    alpha::protort::components::factory::get_component_kind(
                        terminal_.deploy_config_.map_components[component.comp_name].kind);

                // Добавляем компонент к конфигурации нода
                alpha::protort::protocol::deploy::Instance* instance_ = configuration_->add_instances();

                instance_->set_name(component.comp_name);
                instance_->set_kind(kind_);

                // Указываем, что компонент относится к текущему ноду
                alpha::protort::protocol::deploy::Map* components_map_ = configuration_->add_maps();

                components_map_->set_node_name(current_node);
                components_map_->set_instance_name(component.comp_name);

                for (auto &connection : terminal_.deploy_config_.map_component_with_connections[component.comp_name])
                {
                    // Добавляем коннекшион к конфигурации нода
                    alpha::protort::protocol::deploy::Connection* connection_ = configuration_->add_connections();

                    connection_->mutable_source()->set_name(connection.source);
                    connection_->mutable_source()->set_port(connection.source_out);

                    connection_->mutable_destination()->set_name(connection.dest);
                    connection_->mutable_destination()->set_port(connection.dest_in);

                    // Получаем имя нода компонента назначения
                    std::string node_name_ = terminal_.deploy_config_.map_component_node[connection.dest].node_name;

                    if(node_name_ != current_node)
                    {
                        alpha::protort::parser::node& node_ = terminal_.deploy_config_.map_node[node_name_];

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
                         QMetaObject::invokeMethod(
                         this->shared_from_this(),
                         "on_connected_finished",
                         Qt::QueuedConnection,
                         Q_ARG(alpha::protort::protocol::Packet_Payload, p));
                      }
                );

            client_.async_send_request(payload_, ptr_);
        }
    }

    void on_packet_sent(const boost::system::error_code& err, size_t bytes)
    {

    }

    void on_new_packet(alpha::protort::protocol::Packet_Payload packet_)
    {

    }

private slots:

    void on_status_response(alpha::protort::protocol::deploy::StatusResponse status_)
    {
        terminal_.ui->text_browser_status->insertPlainText("<Название узла - " +
                                                 QString::fromStdString(status_.node_name())
                                                 + ">\n<Время работы - " +
                                                 QString::number(status_.uptime()) + " сек." + ">\n<Количество принятых пакетов - "
                                                 + QString::number(status_.in_packets_count()) +
                                                 " (" + QString::number(status_.in_bytes_count())
                                                 + " байт)" + ">\n<Количество переданных пакетов - "
                                                 + QString::number(status_.out_packets_count()) + " ("
                                                 + QString::number(status_.out_bytes_count())
                                                 + " байт)"+ ">\n\n<Информация о компонентах>\n\n");
        for (int j = 0; j < status_.component_statuses_size(); ++j)
        {
            terminal_.ui->text_browser_status->insertPlainText("<Название компонента - " +
                                                     QString::fromStdString(status_.component_statuses(j).name()) +
                                                     ">\n<Количество принятых пакетов - "
                                                     + QString::number(status_.component_statuses(j).in_packet_count()) +
                                                     ">\n<Количество переданных пакетов - " +
                                                     QString::number(status_.component_statuses(j).out_packet_count()) +
                                                     ">\n\n");
        }
        terminal_.ui->text_browser_status->insertPlainText("\n\n");
    }

    void on_connected_finished(alpha::protort::protocol::Packet_Payload packet_)
    {
        terminal_.ui->textBrowser->insertPlainText(node_name_ + " has been connected.\n");
    }

    void on_start_finished(alpha::protort::protocol::Packet_Payload packet_)
    {
        terminal_.ui->textBrowser->insertPlainText("Configuration on " + node_name_ + " has been started.\n");
    }

    void on_stop_finished(alpha::protort::protocol::Packet_Payload packet_)
    {
        terminal_.ui->textBrowser->insertPlainText("Configuration on " + node_name_ + " has been stopped.\n");
    }

private:

    //! Имя узла, с которым коннектится клиент
    QString node_name_;

    Terminal& terminal_;
};

} // namespace protolink
} // namespace protort
} // namespace alpha

#endif // TERMINAL_CLIENT_H
