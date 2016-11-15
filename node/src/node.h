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
#include "router.h"
#include "parser.h"
#include "components.h"
#include "factory.h"
#include "protocol.pb.h"
#include "deploy.pb.h"
#include "factory.h"

namespace alpha {
namespace protort {
namespace node {

using namespace alpha::protort::link;

static const int default_port = 100;

/*!
 * \brief Класс сетевого узла
 */
class node
{
public:
    using protocol_payload = protocol::Packet::Payload;

    node(const node_settings &settings)
        : client_(*this, service_),
          server_(*this, service_),
          server_for_conf_(*this,service_),
          settings_(settings),
          signals_(service_, SIGINT, SIGTERM)
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

    /*!
     * \brief Уведомление об отправке пакета в канал связи
     * \param err
     * \param bytes
     */
    void on_packet_sent(const boost::system::error_code& err, size_t bytes)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о подключении или безуспешном подключении
     * \param err
     */
    void on_connected(const boost::system::error_code& err)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о приеме сервером новогопакета
     * \param buffer
     * \param nbytes
     */
    void on_new_packet(char const *buffer, size_t nbytes)
    {
        std::string payload(buffer, nbytes);
        // Deploy
    }

    /*!
     * \brief Уведомление о приеме сервером нового подключения
     * \param err
     */
    void on_new_connection(const boost::system::error_code& err)
    {
        // TODO
    }

    void on_new_request(const protocol_payload& payload)
    {
        process_request(payload);
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

        // Начинаем прослушивать порт
        //        server_.listen(
        //            boost::asio::ip::tcp::endpoint
        //                (boost::asio::ip::tcp::v4(),
        //                 порт);

        // Создаем отображение имени компонента на информацию о узле
        std::map<std::string, node_info> comp_to_node;

        {
            std::map<std::string, node_info> nodes;

            for (const auto& node : conf.nodes)
                nodes.emplace(node.name, node_info{node.name, node.address, node.port});

            for (const auto& mapp : conf.mappings)
                comp_to_node.emplace(mapp.comp_name, nodes[mapp.node_name]);
        }

        const auto& current_node_name = settings_.name;

        // Создаем экземпляры локальных компонентов
        for (const auto& comp : conf.components) {
            if (comp_to_node[comp.name].name == current_node_name) {
                // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
                router_.component_ptrs.push_back(alpha::protort::components::factory::create(comp.kind));
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
                if (dest_node_name == current_node_name) {
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
                        std::unique_ptr<link::client<node>> client_ptr(new link::client<node>(*this, service_, ep));
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
    }



private:
    protocol_payload process_request(const protocol_payload& payload)
    {
        using PayloadCase = alpha::protort::protocol::Packet_Payload::PayloadCase;

        switch (payload.Payload_case()) {

        case PayloadCase::kCommunicationPacket:
            return process_deploy_request(payload.deploy_packet());

        case PayloadCase::kDeployPacket:
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
            deploy(convert_config(packet.request().deploy_config().config()));
            // TODO
            return {};
        case protocol::deploy::PacketKind::Start:
        default:
            assert(false);
        }
    }

    alpha::protort::parser::configuration convert_config(protocol::deploy::Config config)
    {
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
    link::server<node> server_for_conf_;

    //! Сервер
    link::server<node> server_;

    //! Клиент
    link::client<node> client_;

    //! Настройки узла
    node_settings settings_;

    //! Подписанные сигналы
    boost::asio::signal_set signals_;

public:
    //! Роутер пакетов
    //TODO (ПЕРЕНЕСТИ в private после реализации public методов для использования роутера)
    router<node> router_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
