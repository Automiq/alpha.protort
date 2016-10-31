#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <map>
#include <vector>

#include "packet.pb.h"
#include "client.h"
#include "i_component.h"

namespace alpha {
namespace protort {
namespace node {

using component_ptr = alpha::protort::components::i_component *;
using port_id = alpha::protort::components::port_id;

/*!
 * \brief The node_router class используется для роутинга
 */
template<class node>
class router
{
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
        link::client<node> * client;
    };

    /*!
     * \brief Класс маршрутов
     */
    class routes
    {
    public:
        //! Список локальных маршрутов
        std::vector<local_route> local_components;

        //! Список удаленных маршрутов
        std::vector<remote_route> remote_components;
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

                //
                for (auto &local_component : port_routes.local_components)
                    do_route(local_component.connection, local_component.port, output.payload);

                for (auto &iter_remote_component : port_routes.remote_components)
                {
                    alpha::protort::protocol::Packet packet_;
                    alpha::protort::protocol::ComponentEndpoint out_ep;
                    alpha::protort::protocol::ComponentEndpoint in_ep;

                    // out endpoint
                    out_ep.set_port(static_cast<uint32_t>(out_port));
                    out_ep.set_name(this_component->name);
                    packet_.set_allocated_source(&out_ep);

                    // in endpoint
                    in_ep.set_port(static_cast<uint32_t>(iter_remote_component.port));
                    in_ep.set_name(iter_remote_component.name);
                    packet_.set_allocated_source(&in_ep);

                    // payload
                    packet_.set_payload(output.payload);

                    iter_remote_component.client_->async_send(packet_.SerializeAsString());
                }
            }
        }
    }

    std::map<std::string, component_instance> components;
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // ROUTER_H
