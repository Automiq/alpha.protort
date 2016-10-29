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

using component = alpha::protort::components::i_component;
using component_ptr = component *;
using port_id = unsigned short;

//template<class node> class outport_connections;

///*!
// * \brief The local_input class содержит входной порт и указатель на outport_connections для соединения.
// *  Используется для локального компонента.
// */
//template<class node>
//class local_input
//{
//public:
//    port_id port;
//    outport_connections<node> * connection;
//};

///*!
// * \brief The remote_input class содержит входной порт, имя компонента и указатель на клиента.
// *  Используется для удаленного компонента.
// */
//template<class node>
//class remote_input
//{
//public:
//    port_id port;
//    std::string name;
//    link::client<node> * client_;
//};

///*!
// * \brief The connections class содержит списки локальных у удаленных соединений
// *  для одного выходного порта.
// */
//template<class node>
//class connections
//{
//public:
//    std::vector<local_input> local_components;
//    std::vector<remote_input<node>> remote_components;
//};

///*!
// * \brief The outport_connections class содержит указатель на компонент, имя компонента и
// *  map всех выходных портов с его соединениями.
// */
//template<class node>
//class outport_connections
//{
//public:
//    component_ptr component_;
//    std::string name;
//    std::map<port_id,connections<node>> map_all_connections;
//};

/*!
 * \brief The node_router class используется для роутинга.
 */
template<class node>
class node_router
{
    class outport_connections;

    /*!
     * \brief The local_input class содержит входной порт и указатель на outport_connections для соединения.
     *  Используется для локального компонента.
     */
    class local_input
    {
    public:
        port_id port;
        outport_connections * connection;
    };

    /*!
     * \brief The remote_input class содержит входной порт, имя компонента и указатель на клиента.
     *  Используется для удаленного компонента.
     */
    class remote_input
    {
    public:
        port_id port;
        std::string name;
        link::client<node> * client_;
    };

    /*!
     * \brief The connections class содержит списки локальных у удаленных соединений
     *  для одного выходного порта.
     */
    class connections
    {
    public:
        std::vector<local_input> local_components;
        std::vector<remote_input> remote_components;
    };

    /*!
     * \brief The outport_connections class содержит указатель на компонент, имя компонента и
     *  map всех выходных портов с его соединениями.
     */
    class outport_connections
    {
    public:
        component_ptr component_;
        std::string name;
        std::map<port_id,connections> map_all_connections;
    };

public:
    node_router()
    {

    }

    void do_process(const std::string& component_name,port_id port,const std::string& payload)
    {
        outport_connections<node> comp_outportconnect = component_list[name];
        do_component_process(&comp_outportconnect,port,payload);
    }

    std::map<std::string, outport_connections> component_list;

private:
    void do_component_process(outport_connections* this_component,port_id port,const std::string& payload)
    {
        std::vector<output> output_result = this_component->component_->process(payload,port);

        for (auto &iter_payload : output_result)
        {
            for (auto &iter_output_port : iter_payload.ports)
            {
                connections& port_connections = this_component->map_all_connections[iter_output_port];

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
                    packet_.set_allocated_source(out_ep);

                    // in endpoint
                    in_ep.set_port(static_cast<uint32_t>(iter_remote_component.port));
                    in_ep.set_name(iter_remote_component.name);
                    packet_.set_allocated_source(in_ep);

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
