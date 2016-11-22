#ifndef DEPLOY_CONFIGURATION_H
#define DEPLOY_CONFIGURATION_H

#include <iostream>
#include <unordered_map>
#include <map>

#include "parser.h"

namespace alpha {
namespace protort {
namespace parser {

struct deploy_configuration
{
    std::unordered_map<std::string,mapping> map_component_node;

    std::unordered_map<std::string,component> map_components;
    std::unordered_map<std::string,std::vector<connection>> map_component_with_connections;

    std::unordered_map<std::string,node> map_node;
    std::unordered_map<std::string,std::vector<mapping>> map_node_with_components;

    void parse_deploy(configuration& config)
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

        for (auto &node : config.nodes)
            map_node[node.name] = node;

        for (auto &mapping : config.mappings)
        {
            map_node_with_components[mapping.node_name].push_back(mapping);
            map_component_node[mapping.comp_name] = mapping;
        }
    }
};

} // namespace parser
} // namespace protort
} // namespace alpha

#endif // DEPLOY_CONFIGURATION_H
