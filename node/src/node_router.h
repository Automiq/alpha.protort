#ifndef NODE_ROUTER_H
#define NODE_ROUTER_H

#include <iostream>
#include <map>
#include <vector>

#include "client.h"
#include "i_component"
#include "packet.pb.h"

namespace alpha {
namespace protort {
namespace node {

using component_ptr = component *;
using component = alpha::protort::components::i_component;

//Эта структура здесь только для наглядности, потом уберу
struct output
{
    std::string payload;
    std::vector<unsigned short> ports;
};

class outport_connections;

class local_input
{
public:
    unsigned short port;
    outport_connections * connection;
};

template<class node>
class remote_input
{
public:
    short port;
    std::string name;
    link::client<node> * client_;
};

class connections
{
public:
    std::vector<local_input> local_components;
    std::vector<remote_input> remote_components;
};

class outport_connections
{
public:
    component_ptr component_;
    std::string name;
    std::map<unsigned short,connections> map_;
};

template<class node>
class node_router
{
public:
    node_router(node* n)
        :node_(n)
    {

    }

    void do_process(const std::string& component_name,unsigned short port,const std::string& payload)
    {
        outport_connections comp_outportconnect = component_list[name];
        do_component_process(&comp_outportconnect,port,payload);
    }

    std::map<std::string, outport_connections> component_list;

private:
    void do_component_process(outport_connections* outport,unsigned short port,const std::string& payload)
    {
        std::vector<output> output_result = outport->component_->process(payload,port);

        for(auto iter1 = output_result.begin();iter1 != output_result.end();iter1++)
        {
            for(auto iter2 = iter1->ports.begin();iter2 != iter1->ports.end();iter2++)
            {
                connections& loc_con_port = outport->map_[(*iter2)];

                for(auto iter3 = loc_con_port.local_components.begin();k < iter3 = loc_con_port.local_components.end();iter3++)
                    do_component_process(
                        iter3->connection,
                        iter3->port,
                        iter1->payload);

                for(auto iter3 = loc_con_port.remote_components.begin();k < iter3 = loc_con_port.remote_components.end();iter3++)
                {
                    alpha::protort::protocol::Packet p;
                    alpha::protort::protocol::ComponentEndpoint out_ep;
                    alpha::protort::protocol::ComponentEndpoint in_ep;

                    // out endpoint
                    out_ep.set_port(static_cast<uint32_t>(*iter2));
                    out_ep.set_name(outport->name);
                    p.set_allocated_source(out_ep);

                    // in endpoint
                    in_ep.set_port(static_cast<uint32_t>(iter3->port));
                    in_ep.set_name(iter3->name);
                    p.set_allocated_source(in_ep);

                    // payload
                    p.set_payload(iter1->payload);

                    iter3->client_->async_send(p.SerializeAsString());
                }
            }
        }
    }

    node* node_;
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // NODE_ROUTER_H
