#ifndef NODE_DEPLOY_H
#define NODE_DEPLOY_H

#include <map>
#include <vector>
#include "parser.h"
#include "node.h"
#include <boost/shared_ptr.hpp>
#include "i_component"

std::string current_node_name = "current_node";
using namespace alpha::protort;
using xml_config = parser::configuration;

struct destination
{
    std::string component_name;
    unsigned short input_port;
};

struct component_info
{
    std::string name;
    std::string kind;
    std::string node_name;
    std::map<unsigned short, std::vector<destination> > connections;
};

struct node_info
{
    std::string name;
    std::string address;
    unsigned short port;
    std::vector<component_info*> local_components;
    std::vector<component_info*> remote_components;
};

class node_deploy
{
public:
    void get_node_config(xml_config& conf);
    void deploy();

private:
    node::node* node_;
    std::map<std::string, component_info> components_;
    node_info node_info_;
    std::map<std::string, node_info> nodes_;
};

void node_deploy::get_node_config(xml_config &conf)
{
    for (auto iter = conf.components.begin(); iter != conf.components.end(); ++iter){
        component_info comp_info;
        comp_info.name = iter->name;
        comp_info.kind = iter->kind;
        components_[comp_info.name] = comp_info;
    }
    for (auto iter = conf.connections.begin(); iter != conf.connections.end(); ++iter){
        component_info& comp_info = components_[iter->source_name];
        std::vector<destination>& v_dest =  comp_info.connections[iter->source_out];
        v_dest.push_back( {iter->dest_name, iter->dest_in} );
    }
    for (auto iter = conf.mappings.begin(); iter != conf.mappings.end(); ++iter){
        component_info& comp_info = components_[iter->comp_name];
        comp_info.node_name = iter->node_name;
    }
    for (auto iter = conf.nodes.begin(); iter != conf.nodes.end(); ++iter){
        if (iter->name == current_node_name) {
            node_info_.name = current_node_name;
            for (auto & entry : components_){
                if (entry.second.node_name == current_node_name){
                    node_info_.local_components.push_back(&(entry.second));
                }
                else {
                    node_info_.remote_components.push_back(&(entry.second));
                }
            }
        }
        else {
            nodes_[iter->name] = {iter->name, iter->address, iter->port, {}, {} };
        }
    }
}

void node_deploy::deploy()
{
    for (auto iter = node_info_.local_components.begin(); iter != node_info_.local_components.end(); iter++){
        boost::shared_ptr ptr_to_comp(new alpha::protort::i_component)
        node_->router_.component_list[iter->name] = {}

    }

}

#endif // NODE_DEPLOY_H
