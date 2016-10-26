#ifndef NODE_ROUTER_H
#define NODE_ROUTER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "i_component"
#include "packet.pb.h"


using component_ptr = component *;
using component = alpha::protort::components::i_component;

//Эта структура здесь только для наглядности, потом уберу
struct output
{
    std::string payload;
    std::vector<short> ports;
};

class outport_connections
{
public:
    component_ptr component_;
    std::map<short,outport_connections *> map_;
};

class node_router
{
public:
    void do_process(const std::string& component_name,const std::string& payload)
    {
        outport_connections comp_outportconnect = component_list[name];
        do_component_process(&comp_outportconnect,payload);
    }

    std::map<std::string, outport_connections> component_list;

private:
    void do_component_process(outport_connections* outport,const std::string& payload)
    {
        std::vector<output> result_ = outport->component_->process(payload);

        for(short i = 0;i < result_.size();i++)
            for(short j = 0;j < result_[i].ports.size();j++)
                do_component_process(outport->map_[result_[i].ports[j]],result_[i].payload);
    }
};

#endif // NODE_ROUTER_H
