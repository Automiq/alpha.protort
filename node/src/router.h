#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <map>
#include <vector>

#include "packet.pb.h"
#include "client.h"
#include "component.h"
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
    router()
    {

    }

    /*!
     * \brief Обрабатывает пакет согласно таблице маршрутизации
     * \param component_name Идентификатор компонента
     * \param port Идентификатор входящего порта
     * \param payload Содержимое пакета
     */
    void route(const std::string& component_name, port_id in_port, const std::string& payload)
    {
        do_route(&components[component_name], in_port, payload);
    }

private:
    void do_route(component_instance* this_component, port_id port, const std::string& payload)
    {
        std::vector<alpha::protort::components::output> output_result = this_component->component_->process(port,payload);

        for (auto &output : output_result)
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
                    do_route(local_route.component, local_route.in_port, output.payload);
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
                }
            }
        }
    }

    std::vector<component_unique_ptr> component_ptrs;
    std::map<std::string, component_instance> components;
    std::map<std::string, std::unique_ptr<protolink::client<app>>> clients;
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // ROUTER_H
