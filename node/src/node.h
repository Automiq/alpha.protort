#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/make_shared.hpp>

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

namespace alpha {
namespace protort {
namespace node {

using namespace alpha::protort::protolink;

/*!
 * \brief Класс сетевого узла
 */
class node : public boost::enable_shared_from_this<node>
{
public:
    using protocol_payload = protocol::Packet::Payload;

    node()
        : server_(*this, service_),
          server_for_conf_(*this,service_),
          signals_(service_, SIGINT, SIGTERM),
          router_(boost::make_shared<router<node>>(service_))
    {
    }

    node(const node_settings &settings)
        : server_(*this, service_),
          server_for_conf_(*this,service_),
          settings_(settings),
          signals_(service_, SIGINT, SIGTERM),
          router_(boost::make_shared<router<node>>(service_))
    {
    }

    ~node()
    {
        stop();
    }

    //! Запускает сетевой узел
    void start()
    {
        start_time_ = boost::chrono::steady_clock::now();
        signals_.async_wait(boost::bind(&boost::asio::io_service::stop, &service_));
        server_for_conf_.listen(
                    boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::tcp::v4(),
                     settings_.configuration_port));
        for (int i = 0; i != settings_.threads; i++)
            workers_.create_thread([this](){ service_.run(); });
        service_.run();
    }

    //! Останавливает работу роутера и I/O сервиса
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
    void deploy_from_config(const alpha::protort::parser::configuration& conf)
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

            for (const auto& node : conf.nodes)
                nodes.emplace(node.name, node_info{node.name, node.host.ip_address, node.host.port});

            for (const auto& mapp : conf.mappings)
                comp_to_node.emplace(mapp.comp_name, nodes[mapp.node_name]);
        }

        // Создаем экземпляры локальных компонентов
        for (const auto& comp : conf.components) {
            if (comp_to_node[comp.name].name == node_name_) {
                // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
                component_shared_ptr new_comp = alpha::protort::components::factory::create(comp.kind, router_);
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
                        const auto& n_info = comp_to_node[conn.dest];
                        boost::asio::ip::address_v4 addr(boost::asio::ip::address_v4::from_string(n_info.address));
                        boost::asio::ip::tcp::endpoint ep(addr, n_info.port);
                        auto client_ptr = boost::make_shared<protolink::client<node>>(this->shared_from_this(), service_);
                        client_ptr->async_connect(ep);
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client_ptr}
                                    );
                        router_->clients_[dest_node_name] = client_ptr;
                    }
                    else {
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client->second}
                                    );
                    }
                }
            }
        }

        // Начинаем прослушивать порт
        server_.listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
    }



private:
    protocol_payload process_request(const protocol_payload& payload)
    {
        using PayloadCase = alpha::protort::protocol::Packet_Payload::PayloadCase;

        switch (payload.Payload_case()) {

        case PayloadCase::kCommunicationPacket:
        case PayloadCase::kDeployPacket:
            return process_deploy_request(payload.deploy_packet());
        case PayloadCase::kPayload:
        case PayloadCase::kAnyPayload:
        default:
            assert(false);
            return protocol_payload();
        }
    }

    protocol_payload process_deploy_request(const protocol::deploy::Packet& packet)
    {
        switch (packet.kind()) {

        case protocol::deploy::PacketKind::DeployConfig:
        {
            bool router_previous_state = router_->started_;
            boost::shared_ptr<router<node>> new_router = boost::make_shared<router<node>>(service_);
            auto old_router = boost::atomic_exchange(&router_, new_router);
            deploy_from_packet(packet.request().deploy_config().config());
            if (router_previous_state)
                router_->start();
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
        default:
            assert(false);
        }
    }

    protocol_payload status_response()
    {
        protocol_payload response;
        protocol::deploy::Packet* response_packet = response.mutable_deploy_packet();
        response_packet->set_kind(protocol::deploy::PacketKind::GetStatus);

        response_packet->mutable_response()->mutable_status()->set_node_name(node_name_);

        boost::chrono::duration<double> uptime_period = boost::chrono::steady_clock::now() - start_time_;
        uint32_t uptime = uptime_period.count();
        response_packet->mutable_response()->mutable_status()->set_uptime(uptime);

        response_packet->mutable_response()->mutable_status()->set_in_bytes_count(router_->in_bytes_);
        response_packet->mutable_response()->mutable_status()->set_out_bytes_count(router_->out_bytes_);
        response_packet->mutable_response()->mutable_status()->set_in_packets_count(router_->in_packets_);
        response_packet->mutable_response()->mutable_status()->set_out_packets_count(router_->out_packets_);

        for (auto & component : router_->components_) {
            auto comp_status = response_packet->mutable_response()->mutable_status()->mutable_component_statuses()->Add();
            comp_status->set_in_packet_count(component.second.component_->in_packet_count());
            comp_status->set_out_packet_count(component.second.component_->in_packet_count());
            comp_status->set_name(component.first);
        }

        return response;
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

        for (auto & inst : config.instances())
            pconf.components.push_back({inst.name(), components::get_component_kind(inst.kind())});

        for (auto & conn : config.connections())
            pconf.connections.push_back({conn.source().name(), conn.source().port(),
                                         conn.destination().name(), conn.destination().port()});

        for (auto & node : config.node_infos())
            pconf.nodes.push_back({node.name(), node.address(), node.port()});

        for (auto & map : config.maps())
            pconf.mappings.push_back({map.instance_name(), map.node_name()});

        deploy_from_config(pconf);
    }

    //! I/O сервис
    boost::asio::io_service service_;

    //! Сервер
    protolink::server<node> server_for_conf_;

    //! Сервер
    protolink::server<node> server_;

    //! Настройки узла
    node_settings settings_;

    //! Подписанные сигналы
    boost::asio::signal_set signals_;

    //! Имя узла
    std::string node_name_;

    //! Порт, прослушиваемый сервером узла
    port_id port_;

    //! Время запуска узла
    boost::chrono::steady_clock::time_point start_time_;

    boost::thread_group workers_;

public:
    //! Роутер пакетов
    //TODO (ПЕРЕНЕСТИ в private после реализации public методов для использования роутера)
    boost::shared_ptr<router<node>> router_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
