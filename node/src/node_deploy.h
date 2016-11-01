#ifndef NODE_DEPLOY_H
#define NODE_DEPLOY_H

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "node.h"
#include "parser.h"
#include "components.h"

namespace alpha {
namespace protort {
namespace node {

std::string current_node_name = "current_node";

using xml_config = parser::configuration;

using alpha::protort::components::i_component;
using alpha::protort::components::generator;
using alpha::protort::components::retranslator;
using alpha::protort::components::terminator;
using alpha::protort::node::node;
//using boost::asio::ip;


struct destination
{
    std::string comp_name;
    unsigned short in_port;
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
    void deploy(alpha::protort::node::node &);

private:
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

void node_deploy::deploy(alpha::protort::node::node &_node)
{
    // Создаем экземпляры локальных компонентов
    for (auto const & comp_info_ptr : node_info_.local_components){
        i_component* comp_ptr;

        if (comp_info_ptr->kind == "generator")
            comp_ptr = new generator;
        else if (comp_info_ptr->kind =="retranslator")
            comp_ptr = new retranslator;
        else if (comp_info_ptr->kind =="terminator")
            comp_ptr = new terminator;
        else
            assert(false);

        // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
        _node.router_.components[comp_info_ptr->name] = {comp_ptr, comp_info_ptr->name, {} };
    }

    // Копируем локальные маршруты для каждого экземпляра
    for (auto const & comp_info_ptr : node_info_.local_components){
        router<node>::component_instance & comp_inst = _node.router_.components[comp_info_ptr->name];
        for (auto const & conn : comp_info_ptr->connections){
            for(auto const & dest : conn.second)     {
                comp_inst.port_to_routes[conn.first].local_routes.push_back( {dest.in_port, &comp_inst } );
            }
        }
    }

    // Копируем удаленные маршруты
/*
    for (auto const & comp_info_ptr : node_info_.remote_components){
        // Находим данные узла, на котором размещен удаленный компонент
        node_info& _node_info = nodes_[comp_info_ptr->node_name];
        ip::tcp::endpoint ep(ip::address_v4::from_string(_node_info.address), _node_info.port);

        // Создаем клиентское подключение к удаленному узлу
        _node.router_.clients.push_back( {_node, _node.service_, ep } );

        //Копируем маршрут для соответствующего компонента
        router<node>::component_instance & comp_inst = _node.router_.components[comp_info_ptr->name];
        comp_inst.port_to_routes[]

    }
*/
}

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_DEPLOY_H
