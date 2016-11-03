#ifndef NODE_DEPLOY_H
#define NODE_DEPLOY_H

#include <iostream>
#include <map>
#include <vector>
#include "node.h"
#include "../../parser/src/parser.h"
#include "components.h"

namespace alpha {
namespace protort {
namespace node {

using alpha::protort::node::node;

/*!
 * \brief Разворачивает узел
 * Принимает конфигурацию приложения и развертывания и инициализирует роутер,
 * создавая необходимые компоненты, локальные и удаленные связи.
 */
class node_deploy
{
public:
    /// Маршрут
    struct destination
    {
        std::string comp_name;
        unsigned short in_port;
    };

    /// Информация о компоненте
    struct component_info
    {
        std::string kind;
        std::string node_name;
        std::map<unsigned short, std::vector<destination> > connections;
    };

    /// Информация о узле
    struct node_info
    {
        std::string address;
        unsigned short port;
    };
    /*!
     * \brief Преобразует конфигурацию в промежуточную структуру
     * \param conf Конфигурация, создаваемая xml парсером
     */
    void load_config(alpha::protort::parser::configuration &conf)
    {
        for (const auto & comp : conf.components){
            component_info comp_info;
            comp_info.kind = comp.kind;
            components_[comp.name] = comp_info;
        }

        for (const auto & node : conf.nodes){
            nodes_[node.name] = { node.address, node.port };
        }

        for (const auto & conn : conf.connections){
            component_info& comp_info = components_[conn.source_name];
            std::vector<destination>& v_dest =  comp_info.connections[conn.source_out];
            v_dest.push_back( {conn.dest_name, conn.dest_in} );
        }

        for (const auto & mapp : conf.mappings){
            component_info& comp_info = components_[mapp.comp_name];
            comp_info.node_name = mapp.node_name;
        }
    }

    /*!
     * \brief Разворачивает узел
     * \param _node Узел, на котором происходит разворачивание
     * Создает необходимые компоненты, локальные и удаленные связи роутера.
     */
    void deploy(node &_node)
    {
        // Создаем экземпляры локальных компонентов
        for (const auto & name_to_comp : components_){
            if (name_to_comp.second.node_name == current_node_name){
                component_unique_ptr comp_ptr;

                if (name_to_comp.second.kind == "generator")
                    comp_ptr.reset(new alpha::protort::components::generator);
                else if (name_to_comp.second.kind =="retranslator")
                    comp_ptr.reset( new alpha::protort::components::retranslator);
                else if (name_to_comp.second.kind =="terminator")
                    comp_ptr.reset(new alpha::protort::components::terminator);
                else
                    assert(false);

                // Добавляем ссылки на экземпляры в таблицу маршрутов роутера
                _node.router_.components_.push_back(std::move(comp_ptr));
                _node.router_.components[name_to_comp.first] = {_node.router_.components_.back().get(), name_to_comp.first, {} };
            }
        }

        // Копируем маршруты для каждого локального компонента
        for (auto & name_to_comp : _node.router_.components){
            const std::string & inst_name = name_to_comp.first;
            router<node>::component_instance & comp_inst = name_to_comp.second;

            // Копируем маршруты компонента
            component_info& comp_info = components_[inst_name];

            // Для каждого набора соединений
            for (const auto & port_to_conn : comp_info.connections){
                port_id out_port = port_to_conn.first;

                // Для каждого получателя
                for(const auto & dest : port_to_conn.second){
                    std::string dest_node_name = components_[dest.comp_name].node_name;

                    // Если получатель локальный                    
                    if (dest_node_name == current_node_name){
                        router<node>::component_instance * dest_ptr = &(_node.router_.components[dest.comp_name]);
                        comp_inst.port_to_routes[out_port].local_routes.push_back( {dest.in_port, dest_ptr } );
                    }

                    //Если получатель удаленный
                    else {
                        //Если нет клиента для удаленного узла, то создаем соответствующий
                        auto _client = _node.router_.clients.find(dest_node_name);
                        if (_client == _node.router_.clients.end()){
                            node_info n_info = nodes_[dest_node_name];
                            boost::asio::ip::address_v4 addr { boost::asio::ip::address_v4::from_string(n_info.address) };
                            boost::asio::ip::tcp::endpoint ep(addr, n_info.port);
                           std::unique_ptr<link::client<node> > cl( new link::client<node> { _node, _node.service_, ep } );
                           router<node>::remote_route rem_route {dest.in_port, dest.comp_name, cl.get() };
                           comp_inst.port_to_routes[out_port].remote_routes.push_back(rem_route);
                            _node.router_.clients[dest_node_name] = std::move(cl);

                        }
                        else {
                            router<node>::remote_route rem_route {dest.in_port, dest.comp_name, _client->second.get() };
                            comp_inst.port_to_routes[out_port].remote_routes.push_back(rem_route);
                        }
                    }
                }
            }
        }
    }

private:
    std::map<std::string, component_info> components_;    
    std::map<std::string, node_info> nodes_;
    const std::string current_node_name = "current_node";
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_DEPLOY_H
