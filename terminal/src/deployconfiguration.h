#ifndef DEPLOY_CONFIGURATION_H
#define DEPLOY_CONFIGURATION_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <boost/format.hpp>

#include "parser.h"

struct deploy_configuration
{
    std::unordered_map<std::string, alpha::protort::parser::mapping> map_component_node;
    std::unordered_map<std::string, alpha::protort::parser::component> map_components;
    std::unordered_map<std::string, alpha::protort::parser::node> map_node;
    std::unordered_map<std::string, std::vector<alpha::protort::parser::mapping>> map_node_with_components;
    std::unordered_map<std::string, std::vector<alpha::protort::parser::connection>> map_component_with_connections;

    void parse_deploy(alpha::protort::parser::configuration& config)
    {
        map_component_node.clear();

        map_components.clear();
        map_component_with_connections.clear();

        map_node.clear();
        map_node_with_components.clear();

        for (auto &component : config.components)
            map_components[component.name] = component;

        for (auto &connection : config.connections)
            map_component_with_connections[connection.source].push_back(connection);

        for (auto &node : config.nodes){
           if(node.pairnode){
               alpha::protort::parser::node tmp_pairnode;
               tmp_pairnode.name = (boost::format("pairnode.%1%") % node.name).str();
               tmp_pairnode.host = node.pairnode.get();
               map_node[tmp_pairnode.name] = tmp_pairnode;
           }
           map_node[node.name] = node;
        }

        for (auto &mapping : config.mappings){
            if(map_node[mapping.node_name].pairnode){
               alpha::protort::parser::mapping tmp_mapping_pairnode;
               tmp_mapping_pairnode.comp_name = mapping.comp_name;
               tmp_mapping_pairnode.node_name = (boost::format("pairnode.%1%") % mapping.node_name).str();
               map(tmp_mapping_pairnode);
            }
            map(mapping);
        }
    }

private:
    void map(alpha::protort::parser::mapping const &mapping)
    {
        map_node_with_components[mapping.node_name].push_back(mapping);
        map_component_node[mapping.comp_name] = mapping;
    }
};

#endif // DEPLOY_CONFIGURATION_H
