#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <map>
#include <vector>
#include <boost/bind.hpp>

#include "packet.pb.h"
#include "client.h"
#include "node.h"


namespace alpha {
namespace protort {
namespace node {
namespace tests {
void test_node_router();
}
}
}
}

namespace alpha {
namespace protort {
namespace node {

using component_ptr = alpha::protort::components::component *;
using component_unique_ptr = std::unique_ptr<alpha::protort::components::component>;
using port_id = alpha::protort::components::port_id;

/*!
 * \brief Роутер пакетов
 */
template<class app>
class router
{
    friend class node;
    friend class alpha::protort::components::component;
    friend void alpha::protort::node::tests::test_node_router();
private:
    class component_instance;

    /*!
     * \brief Класс локального маршрута
     * Используется для адресации локального компонента.
     */
    class local_route
    {
    public:
        //! Входной порт адресуемого компонента
        port_id in_port;

        //! Указатель на экземпляр адресуемого компонента
        component_instance * component;
    };

    /*!
     * \brief Класс удаленного маршрута
     * Используется для адресации удаленного компонента.
     */
    class remote_route
    {
    public:
        //! Входной порт адресуемого компонента
        port_id in_port;

        //! Идентификатор компонента
        std::string name;

        //! Указатель на клиентское подключение
        protolink::client<app> * client;
    };

    /*!
     * \brief Класс маршрутов
     */
    class routes
    {
    public:
        //! Список локальных маршрутов
        std::vector<local_route> local_routes;

        //! Список удаленных маршрутов
        std::vector<remote_route> remote_routes;
    };

    /*!
     * \brief Экземпляр компонента
     * Класс используется для маршрутизации пакетов между компонентами в рамках графа,
     * описывающего структуру приложения. Содержит необходимую информацию о компоненте
     * для его идентификации внутри графа.
     */
    class component_instance
    {
    public:
        //! Указатель на объект компонента
        component_ptr component_;

        //! Идентификтор экземпляра компонента
        std::string name;

        //! Таблица маршрутов
        std::map<port_id, routes> port_to_routes;
    };

public:
    router(boost::asio::io_service& service): service(service)
    {

    }

    void start()
    {
        started = true;
        for (auto & comp : component_ptrs) {
            comp->start();
        }
    }

    void stop()
    {
        started = false;
        for (auto & comp : component_ptrs) {
            comp->stop();
        }
    }

    void clear()
    {
        component_ptrs.clear();
        components.clear();
        clients.clear();
    }

    /*!
     * \brief Обрабатывает пакет согласно таблице маршрутизации
     * \param component_name Идентификатор компонента
     * \param port Идентификатор входящего порта
     * \param payload Содержимое пакета
     */
    void route(const std::string& component_name, port_id in_port, const std::string& payload)
    {
        auto it = components.find(component_name);
        if (it != components.end())
        {
            service.post(boost::bind(&protort::components::component::process,
                                     it->second.component_,
                                     in_port,
                                     payload));
            in_bytes += sizeof(payload);
            in_packets++;
        }
    }

    void do_route(void *comp_inst,
                  const std::vector<alpha::protort::components::output>& outputs)
    {
        if (comp_inst == nullptr || !started)
        {
            std::cout << "nullptr or router::!started" << std::endl;
            assert(false);
            return;
        }
        component_instance* this_component = static_cast<component_instance*>(comp_inst);

        for (auto &output : outputs)
        {
            for (auto const &out_port : output.ports)
            {
                routes& port_routes = this_component->port_to_routes[out_port];

                // Рассылаем пакеты по локальным маршрутам
                for (auto &local_route : port_routes.local_routes)
                {

                    std::cout << "using do_route: \nfrom comp " << this_component->name
                              << " out port " << out_port << std::endl;
                    std::cout << "to comp " << local_route.component->name
                              << " in port " << local_route.in_port << std::endl;
                    service.post(boost::bind(&protort::components::component::process,
                                             local_route.component->component_,
                                             local_route.in_port,
                                             output.payload));
                }

                // Формируем и рассылаем пакеты по удаленным маршрутам
                for (auto &remote_route : port_routes.remote_routes)
                {
                    alpha::protort::protocol::Packet_Payload payload;
                    auto packet = payload.mutable_communication_packet();

                    // out endpoint
                    packet->mutable_source()->set_port(static_cast<uint32_t>(out_port));
                    packet->mutable_source()->set_name(this_component->name);

                    // in endpoint
                    packet->mutable_destination()->set_port(static_cast<uint32_t>(remote_route.in_port));
                    packet->mutable_destination()->set_name(remote_route.name);

                    // payload
                    packet->set_payload(output.payload);

                    remote_route.client->async_send_message(payload);
                    std::cout << "Sending packet to " << remote_route.name << std::endl;
                    out_bytes += sizeof(payload);
                    out_packets++;
                }
            }
        }
    }

    boost::asio::io_service& get_service()
    {
        return service;
    }

private:
    std::vector<component_unique_ptr> component_ptrs;
    std::map<std::string, component_instance> components;
    std::map<std::string, std::unique_ptr<protolink::client<app>>> clients;
    boost::asio::io_service& service;
    bool started = false;
    uint32_t in_bytes = 0;
    uint32_t out_bytes = 0;
    uint32_t in_packets = 0;
    uint32_t out_packets = 0;
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // ROUTER_H
