#ifndef NODE_DEPLOY_H
#define NODE_DEPLOY_H

#include <map>
#include <vector>
#include "parser.h"

using alpha::protort::parser;
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
    std::map<unsigned short, vector<destination> > connections;
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
    xml_config get_node_config(xml_config& conf);
    void deploy();

private:
    xml_config node_config_;
    std::vector<parser::component> components_;

};

node_deploy::deploy()
{
    for (auto comp = node_config_.components.begin(); comp != node_config_.components.end(); comp++){

    }

}

#endif // NODE_DEPLOY_H
