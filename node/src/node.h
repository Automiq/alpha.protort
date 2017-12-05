#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "server.h"
#include "client.h"
#include "node_settings.h"
#include "packet.pb.h"
#include "components.h"
#include "router.h"
#include "parser.h"
#include "factory.h"
#include "protocol.pb.h"
#include "deploy.pb.h"
#include "factory.h"
#include "backup_manager.h"
#include "logi.h"

using namespace alpha::protort;

namespace alpha {
namespace protort {
namespace node {

/*!
 * \brief Класс сетевого узла
 */
class node : public boost::enable_shared_from_this<node>
{
public:
    using protocol_payload = protocol::Packet::Payload;
    using client_t = protolink::client<node>;
    using client_ptr = boost::shared_ptr<client_t>;

    /*!
     * \brief Конструктор по умолчанию
     */
    node()
        : server_(*this, service_),//создает сервер
          server_for_conf_(*this,service_),//создает сервер
          signals_(service_, SIGINT, SIGTERM),// объект для прослушивания сигналов
          router_(boost::make_shared<router<node>>(service_))//создает роутер пакетов

    {
    }

    /*!
     * \brief Конструктор с использование конфигурации
     */
    node(const node_settings &settings)
        : server_(*this, service_),
          server_for_conf_(*this,service_),
          settings_(settings),// использует конфигурацию
          signals_(service_, SIGINT, SIGTERM),
          router_(boost::make_shared<router<node>>(service_))
    {
    }

    /*!
     * \brief Деструктор
     */
    ~node()
    {
        stop();//останавливает работу ноды
    }

    /*!
     * \brief Метод запускает узел
     */
    void start()
    {
        start_time_ = boost::chrono::steady_clock::now();// время запуска.
        signals_.async_wait(boost::bind(&boost::asio::io_service::stop, &service_));// разбрать как работает bind

        // запускает асинхронную операцию, ожидание сигналов
        server_for_conf_.listen(
                    boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::tcp::v4(),
                     settings_.configuration_port));//слушает новые подключения на данном порту

        for (int16_t i = 0; i != settings_.threads; i++){
            workers_.create_thread([this](){ service_.run(); }); // разобрать !!!!!!
        }

        service_.run();
    }

    /*!
     * \brief Метод oстанавливает работу роутера и I/O сервиса
     */
    void stop()
    {
        router_->stop();
        service_.stop();
        workers_.join_all();
    }

    /*!
     * \brief Уведомление об отправке пакета в канал связи
     * \param err
     * \param bytes
     */
    void on_packet_sent(const boost::system::error_code& err, size_t bytes)
    {

    }

    /*!
     * \brief Уведомление о подключении или безуспешном подключении
     * \param err
     */
    void on_connected(const boost::system::error_code& err)
    {

    }

    /*!
     * \brief Уведомление о приеме сервером нового подключения
     * \param err
     */
    void on_new_connection(const boost::system::error_code& err)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о приеме нового сообщения
     * \param payload
     */
    void on_new_message(const protocol_payload& payload)
    {
#ifdef _DEBUG
        std::cout << "node::on_new_message for comp  " << payload.communication_packet().destination().name() << std::endl;
#endif
        //смотрит информацию о сообщение.
        router_->in_bytes_ += payload.ByteSize();
        router_->route(payload.communication_packet().destination().name(),
                      payload.communication_packet().destination().port(),
                      payload.communication_packet().payload());
    }

    /*!
     * \brief Уведомление о приеме нового запроса
     * \param payload
     * \return
     */
    protocol_payload on_new_request(const protocol_payload& payload)
    {
        return process_request(payload);
    }

    /*!
     * \brief Разворачивает узел
     * \param conf Конфигурация полученная парсером из xml
     * Создает необходимые компоненты, локальные и удаленные связи роутера.
     */
    void deploy_from_config(const parser::configuration& conf)
    {
        struct node_info
        {
            std::string name;
            std::string address;
            uint32_t port;
        };

        // Создаем отображение имени компонента на информацию о узле
        std::map<std::string, node_info> comp_to_node;

        {
            std::map<std::string, node_info> nodes;

            for (const auto &node : conf.nodes){
                nodes.emplace(node.name, node_info{node.name, node.host.ip_address, node.host.port});
            }

            for (const auto& mapp : conf.mappings){
                comp_to_node.emplace(mapp.comp_name, nodes[mapp.node_name]);
            }
        }

        // Создаем экземпляры локальных компонентов
        for (const auto &comp : conf.components) {
            if (comp_to_node[comp.name].name == node_name_) {

                // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
                component_shared_ptr new_comp = components::factory::create(comp.kind, router_);

                router_->components_[comp.name] = {new_comp, comp.name, {}};
                new_comp->set_comp_inst(&router_->components_[comp.name]);
            }
        }

        // Для каждого локального компонента
        for (const auto& conn : conf.connections) {
            auto name_to_comp_inst = router_->components_.find(conn.source);

            if (name_to_comp_inst != router_->components_.end()) {
                auto& comp_inst = name_to_comp_inst->second;
                const auto& dest_node_name = comp_to_node[conn.dest].name;

                // Копируем локальный маршрут
                if (dest_node_name == node_name_) {
                    router<node>::component_instance* dest_ptr = &(router_->components_[conn.dest]);

                    comp_inst.port_to_routes[conn.source_out].local_routes.push_back({conn.dest_in, dest_ptr});
                }
                // Копируем удаленный маршрут
                else {
                    // Если нет клиента для удаленного узла, то создаем соответствующий
                    auto client = router_->clients_.find(dest_node_name);

                    if (client == router_->clients_.end()) {
                        const auto &n_info = comp_to_node[conn.dest];

                        boost::asio::ip::address_v4 addr(boost::asio::ip::address_v4::from_string(n_info.address));
                        boost::asio::ip::tcp::endpoint ep(addr, n_info.port);

                        auto client_ptr = boost::make_shared<protolink::client<node>>(this->shared_from_this(), service_ , ep);
                        if(!backup_manager_&&backup_manager_->backup_status()!= protocol::backup::BackupStatus::Slave){
                            client_ptr->async_connect(ep);
                        }
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client_ptr});

                        router_->clients_[dest_node_name] = client_ptr;
                    }
                    else {
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client->second});
                    }
                }
            }
        }

        // Начинаем прослушивать порт
        server_.listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));

        std::cout<<"deploy_stop"<<std::endl;
    }

private:

    /*!
     * \brief Обработчик содержимого пакетов
     */
    protocol_payload process_request(const protocol_payload& payload)
    {
        using PayloadCase = alpha::protort::protocol::Packet_Payload::PayloadCase;

        switch (payload.Payload_case()) {
            case PayloadCase::kCommunicationPacket:

            case PayloadCase::kDeployPacket:
                return process_deploy_request(payload.deploy_packet());

            case PayloadCase::kBackupPacket:
                return process_backup_request(payload.backup_packet());

            case PayloadCase::kPayload:

            case PayloadCase::kAnyPayload:

            default:
                assert(false);
                return protocol_payload();
        }
    }

    /*!
     * \brief  Обработчик backup пакетов
     */
    protocol_payload process_backup_request(const protocol::backup::Packet packet)
    {
        switch(packet.kind()){
            case protocol::backup::PacketType::KeepAlive:
                return backup_manager_->keepalive_response();

            case protocol::backup::PacketType::Switch:
                backup_manager_->backup_transition();
                return{};

            case protocol::backup::PacketType::GetStatus:
                return backup_status_response();

            default:
                assert(false);
                return protocol_payload();
        }
    }

    /*!
     * \brief Обработчик deploy пакетов
     */
    protocol_payload process_deploy_request(const protocol::deploy::Packet& packet)
    {
        switch (packet.kind()) {
            case protocol::deploy::PacketKind::DeployConfig:{
                bool router_previous_state = router_->started_;
                boost::shared_ptr<router<node>> new_router = boost::make_shared<router<node>>(service_);
                auto old_router = boost::atomic_exchange(&router_, new_router);

                deploy_from_packet(packet.request().deploy_config().config());

                if (router_previous_state){
                    router_->start();
                }

                old_router->stop();

                return {};
            }

            case protocol::deploy::PacketKind::Start:
                router_->start();
                return {};

            case protocol::deploy::PacketKind::Stop:
                router_->stop();
                return {};

            case protocol::deploy::PacketKind::GetStatus:
                return status_response();

            case protocol::deploy::PacketKind::Update:


            case protocol::deploy::PacketKind::Switch:
                backup_manager_->backup_transition();
                return{};

            default:
                assert(false);
                return protocol_payload();
        }
    }

    /*!
     * \brief Метод формирует ответ на запрос GetStatus
     */
    protocol_payload status_response()
    {
        protocol_payload response;
        protocol::deploy::Packet* response_packet = response.mutable_deploy_packet();

        response_packet->set_kind(protocol::deploy::PacketKind::GetStatus);
        response_packet->mutable_response()->mutable_status()->set_node_name(node_name_);

        boost::chrono::duration<double> uptime_period = boost::chrono::steady_clock::now() - start_time_;
        uint32_t uptime = static_cast<uint32_t>(uptime_period.count());

        response_packet->mutable_response()->mutable_status()->set_uptime(uptime);
        response_packet->mutable_response()->mutable_status()->set_in_bytes_count(router_->in_bytes_);
        response_packet->mutable_response()->mutable_status()->set_out_bytes_count(router_->out_bytes_);
        response_packet->mutable_response()->mutable_status()->set_in_packets_count(router_->in_packets_);
        response_packet->mutable_response()->mutable_status()->set_out_packets_count(router_->out_packets_);
        response_packet->mutable_response()->mutable_status()->mutable_node_info()->set_backup_status(backup_manager_->backup_status());

        for (auto & component : router_->components_) {
            auto comp_status = response_packet->mutable_response()->mutable_status()->mutable_component_statuses()->Add();

            comp_status->set_in_packet_count(component.second.component_->in_packet_count());
            comp_status->set_out_packet_count(component.second.component_->in_packet_count());
            comp_status->set_name(component.first);
        }

        return response;
    }

    protocol_payload backup_status_response()
    {
        protocol_payload backup_response;
        protocol::backup::Packet* backup_response_packet = backup_response.mutable_backup_packet();

        backup_response_packet->set_kind(protocol::backup::PacketType::GetStatus);
        backup_response_packet->mutable_response()->mutable_status()->set_backup_status(backup_manager_->backup_status());

        return backup_response;
    }
    /*!
     * \brief Разворачивает узел, используя пакет, полученный от терминала
     * \param config Конфигурация из пакета
     * \return Ответный пакет
     * Присваивает узлу имя и прослушиваемый порт в соответсвии с данными,
     * содержащимися в пакете.
     * Разворачивает узел путем преобразования конфигурации из пакета в
     * конфигурацию парсера xml и используя функцию deploy_from_config.
     *
     */
    void deploy_from_packet(const protocol::deploy::Config& config)
    {
        node_name_ = config.this_node_info().name();
        port_ = config.this_node_info().port();

        parser::configuration pconf;

        for (auto & inst : config.instances()){
            pconf.components.push_back({inst.name(), components::get_component_kind(inst.kind())});
        }

        for (auto & conn : config.connections()){
            pconf.connections.push_back({conn.source().name(), conn.source().port(),
                                         conn.destination().name(), conn.destination().port()});
        }

        for (auto & node : config.node_infos()){
            if(node.name() == node_name_ && config.this_node_info().backup_status() != node.backup_status()){
                boost::asio::ip::tcp::endpoint ep(
                            boost::asio::ip::address::from_string(node.address()),
                            node.port()
                            );
                client_ = boost::make_shared<client_t>(this->shared_from_this(), service_ , ep);
                client_->async_start();
                backup_manager_ = boost::make_shared<Backup_manager>(service_,
                                                                    static_cast<alpha::protort::node::Node_status>(config.this_node_info().backup_status()),
                                                                    client_,
                                                                    router_
                                                                    );

                backup_manager_->start_keepalife();
            }
            else{
                pconf.nodes.push_back({node.name(), node.address(), node.port()});
            }
        }

        for (auto & map : config.maps()){
            pconf.mappings.push_back({map.instance_name(), map.node_name()});
        }

        deploy_from_config(pconf);
    }

    //! I/O сервис
    boost::asio::io_service service_;
    //! Сервер
    protolink::server<node> server_for_conf_;
    //! Сервер
    protolink::server<node> server_;
    //! Клиент
    client_ptr client_;

public:
    //! Роутер
    boost::shared_ptr<router<node>> router_;

private:
    //! Настройки узла
    node_settings settings_;

    //! Подписанные сигналы
    boost::asio::signal_set signals_;

    //! Имя узла
    std::string node_name_;

    //! Порт, прослушиваемый сервером узла
    port_id port_;

    //! Менеджер для работы с парой
    boost::shared_ptr<Backup_manager> backup_manager_;

    //! Время запуска узла
    boost::chrono::steady_clock::time_point start_time_;

    boost::thread_group workers_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
