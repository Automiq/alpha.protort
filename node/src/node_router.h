#ifndef NODE_ROUTER_H
#define NODE_ROUTER_H

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
 * \brief The node_router class используется для роутинга.
 */
template<class node_>
class node_router
{
public:
    class component_with_connections;

    /*!
     * \brief The local_input class содержит входной порт и указатель на outport_connections для соединения.
     *  Используется для локального компонента.
     */
    class local_input_
    {
    public:
        port_id port;
        component_with_connections * connection;
    };

    /*!
     * \brief The remote_input class содержит входной порт, имя компонента и указатель на клиента.
     *  Используется для удаленного компонента.
     */
    class remote_input_
    {
    public:
        port_id port;
        std::string name;
        link::client<node_> * client_;
    };

    /*!
     * \brief The connections class содержит списки локальных у удаленных соединений
     *  для одного выходного порта.
     */
    class outport_
    {
    public:
        std::vector<local_input_> local_components;
        std::vector<remote_input_> remote_components;
    };

    /*!
     * \brief The outport_connections class содержит указатель на компонент, имя компонента и
     *  map всех выходных портов с его соединениями.
     */
    class component_with_connections
    {
    public:
        component_ptr component_;
        std::string name;
        std::map<port_id,outport_> map_all_connections;
    };

    node_router()
    {

    }

    void do_process(const std::string& component_name,port_id port,const std::string& payload)
    {
        do_component_process(&component_list[component_name],port,payload);
    }

    std::map<std::string, component_with_connections> component_list;

private:
    void do_component_process(component_with_connections* this_component,port_id port,const std::string& payload)
    {
        std::vector<alpha::protort::components::output> output_result = this_component->component_->process(port,payload);

        for (auto &iter_payload : output_result)
        {
            for (auto &iter_output_port : iter_payload.ports)
            {
                outport_& port_connections = this_component->map_all_connections[iter_output_port];

                for (auto &iter_local_component : port_connections.local_components)
                    do_component_process(
                        iter_local_component.connection,
                        iter_local_component.port,
                        iter_payload.payload);

                for (auto &iter_remote_component : port_connections.remote_components)
                {
                    alpha::protort::protocol::Packet packet_;
                    alpha::protort::protocol::ComponentEndpoint out_ep;
                    alpha::protort::protocol::ComponentEndpoint in_ep;

                    // out endpoint
                    out_ep.set_port(static_cast<uint32_t>(iter_output_port));
                    out_ep.set_name(this_component->name);
                    packet_.set_allocated_source(&out_ep);

                    // in endpoint
                    in_ep.set_port(static_cast<uint32_t>(iter_remote_component.port));
                    in_ep.set_name(iter_remote_component.name);
                    packet_.set_allocated_source(&in_ep);

                    // payload
                    packet_.set_payload(iter_payload.payload);

                    iter_remote_component.client_->async_send(packet_.SerializeAsString());
                }
            }
        }
    }
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // NODE_ROUTER_H
