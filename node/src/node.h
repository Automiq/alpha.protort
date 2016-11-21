#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

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

static const int default_port = 100;

/*!
 * \brief Класс сетевого узла
 */
class node
{
public:
    using protocol_payload = protocol::Packet::Payload;

    node(const node_settings &settings)
        : server_(*this, service_),
          server_for_conf_(*this,service_),
          settings_(settings),
          signals_(service_, SIGINT, SIGTERM),
          router_(service_)
    {
    }

    void start()
    {
        signals_.async_wait(boost::bind(&boost::asio::io_service::stop, &service_));
        server_for_conf_.listen(
                    boost::asio::ip::tcp::endpoint
                    (boost::asio::ip::tcp::v4(),
                     default_port));
        service_.run();
    }

    void stop()
    {
        service_.stop();
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
        std::cout << "node::on_new_message for comp  " << payload.communication_packet().destination().name() << std::endl;
        router_.route(payload.communication_packet().destination().name(),
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
    void deploy(const alpha::protort::parser::configuration& conf)
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
                nodes.emplace(node.name, node_info{node.name, node.address, node.port});

            for (const auto& mapp : conf.mappings)
                comp_to_node.emplace(mapp.comp_name, nodes[mapp.node_name]);
        }

        if (node_name_.empty()) {
            std::cout << "node_name_ is empty" << std::endl;
            node_name_ = settings_.name;
            std::cout << node_name_ << " name was set\n";
        }

        // Создаем экземпляры локальных компонентов
        for (const auto& comp : conf.components) {
            if (comp_to_node[comp.name].name == node_name_) {
                // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
                router_.component_ptrs.push_back
                        (alpha::protort::components::factory::create(comp.kind, router_, comp.name));
                router_.components[comp.name] = {router_.component_ptrs.back().get(), comp.name, {}};
            }
        }

        // Для каждого локального компонента
        for (const auto& conn : conf.connections) {
            auto name_to_comp_inst = router_.components.find(conn.source);
            if (name_to_comp_inst != router_.components.end()) {
                auto& comp_inst = name_to_comp_inst->second;
                const auto& dest_node_name = comp_to_node[conn.dest].name;

                // Копируем локальный маршрут
                if (dest_node_name == node_name_) {
                    router<node>::component_instance* dest_ptr = &(router_.components[conn.dest]);
                    comp_inst.port_to_routes[conn.source_out].local_routes.push_back({conn.dest_in, dest_ptr});
                }
                // Копируем удаленный маршрут
                else {
                    // Если нет клиента для удаленного узла, то создаем соответствующий
                    auto client = router_.clients.find(dest_node_name);
                    if (client == router_.clients.end()) {
                        const auto& n_info = comp_to_node[conn.dest];
                        boost::asio::ip::address_v4 addr(boost::asio::ip::address_v4::from_string(n_info.address));
                        boost::asio::ip::tcp::endpoint ep(addr, n_info.port);
                        std::unique_ptr<protolink::client<node>> client_ptr(new protolink::client<node>(*this, service_, ep));
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client_ptr.get()}
                                    );
                        router_.clients[dest_node_name] = std::move(client_ptr);
                    }
                    else {
                        comp_inst.port_to_routes[conn.source_out].remote_routes.push_back(
                                    router<node>::remote_route{conn.dest_in, conn.dest, client->second.get()}
                                    );
                    }
                }
            }
        }

        // Начинаем прослушивать порт
        if (port_)
            server_.listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
        else
            server_.listen(settings_.source);
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
            deploy(convert_config(packet.request().deploy_config().config()));

            // Формируем ответный пакет
            protocol_payload response;
            protocol::deploy::Packet* response_packet = response.mutable_deploy_packet();
            response_packet->set_kind(protocol::deploy::PacketKind::DeployConfig);
            response_packet->mutable_error()->set_message("deployed");
            return response;
        }

        case protocol::deploy::PacketKind::Start:
            router_.start();
        case protocol::deploy::PacketKind::Stop:
            router_.stop();
        default:
            assert(false);
        }
    }

    alpha::protort::parser::configuration convert_config(protocol::deploy::Config config)
    {
        node_name_ = config.this_node_info().name();
        port_ = config.this_node_info().port();

        parser::configuration pconf;

        for (auto & inst : config.instances()) {
            pconf.components.push_back({inst.name(), components::factory::get_component_kind(inst.kind())});
        }

        for (auto & conn : config.connections()) {
            pconf.connections.push_back({conn.source().name(), conn.source().port(),
                                         conn.destination().name(), conn.destination().port()});
        }

        for (auto & node : config.node_infos()) {
            pconf.nodes.push_back({node.name(), node.address(), node.port()});
        }

        for (auto & map : config.maps()) {
            pconf.mappings.push_back({map.instance_name(), map.node_name()});
        }

        return pconf;
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

public:
    //! Роутер пакетов
    //TODO (ПЕРЕНЕСТИ в private после реализации public методов для использования роутера)
    router<node> router_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
