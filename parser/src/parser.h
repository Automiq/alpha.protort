#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <unordered_map>

namespace alpha {
namespace protort {
namespace parser {

using port_id = uint32_t;

/*!
 * \brief Класс компонент
 *
 * Класс содержит имя и тип компонента
 */
struct component
{
    std::string name;
    std::string kind;
};

/*!
 * \brief Класс связи между портами компонентов
 *
 * Класс хранит имена компонентов и связанные порты
 */
struct connection
{
    std::string source;
    port_id source_out;
    std::string dest;
    port_id dest_in;
};

/*!
 * \brief Класс сетевого узла
 *
 * Класс хранит имя, адрес и порт узла
 */
struct node
{
    std::string name;
    std::string address;
    port_id port;
};

/*!
 * \brief Класс отображения компонента на сетевой узел
 *
 * Класс хранит имя компонента и имя узла
 */
struct mapping
{
    std::string comp_name;
    std::string node_name;
};

/*!
 * \brief Класс, описывающий схему приложения и развертывания
 *
 * Данный класс получает и хранит информацию путем парсинга xml файлов схемы приложения
 * и схемы развертывания.
 */
struct configuration
{
    std::vector<component> components;
    std::vector<connection> connections;
    std::vector<node> nodes;
    std::vector<mapping> mappings;

    std::unordered_map<std::string,mapping*> map_component_node;

    std::unordered_map<std::string,component*> map_components;
    std::unordered_map<std::string,std::vector<connection*>> map_component_with_connections;

    std::unordered_map<std::string,node*> map_node;
    std::unordered_map<std::string,std::vector<mapping*>> map_node_with_components;

    /*!
     * \brief Парсит схему приложения
     * \param путь к файлу схемы приложения
     * \return true если успешно, иначе false
     *
     * Метод парсит схему приложения и хранит полученную информацию внутри класса в соответствующих атрибутах
     * Содержит обработчик ошибок, которые обычно возникают на этапе чтения xml файла, выводит сообщения
     * об ошибках в стандартный вывод.
     */
    bool parse_app(const std::string &filename)
    {
        try
        {
            map_components.clear();
            map_component_with_connections.clear();
            components.clear();
            connections.clear();
            using boost::property_tree::ptree;
            boost::property_tree::ptree pt;
            read_xml(filename, pt);

            BOOST_FOREACH( ptree::value_type const& v, pt.get_child("app") ) {
                if( v.first == "instance" ) {
                    component comp;
                    comp.name = v.second.get<std::string>("<xmlattr>.name");
                    comp.kind = v.second.get<std::string>("<xmlattr>.kind");
                    components.push_back(comp);
                    map_components[comp.name] = &components.back();
                }
                else if( v.first == "connection" ) {
                    connection conn;
                    conn.source = v.second.get<std::string>("<xmlattr>.source");
                    conn.source_out = v.second.get<port_id>("<xmlattr>.source_out");
                    conn.dest = v.second.get<std::string>("<xmlattr>.dest");
                    conn.dest_in = v.second.get<port_id>("<xmlattr>.dest_in");
                    connections.push_back(conn);
                    map_component_with_connections[conn.source].push_back(&connections.back());
                }
                else
                    std::cout << "Unknown tag in the file" << std::endl;
            }
            return true;
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
            return false;
        }
    }

    /*!
     * \brief Парсит схему развертывания
     * \param путь к файлу схемы развертыания
     * \return true если успешно, иначе false
     *
     * Метод парсит схему развертывания и хранит полученную информацию внутри класса в соответствующих атрибутах
     * Содержит обработчик ошибок, которые обычно возникают на этапе чтения xml файла, выводит сообщения
     * об ошибках в стандартный вывод.
     */
    bool parse_deploy(const std::string &filename)
    {
        try
        {
            map_node.clear();
            map_node_with_components.clear();
            map_component_node.clear();
            nodes.clear();
            mappings.clear();
            using boost::property_tree::ptree;
            boost::property_tree::ptree pt;
            read_xml(filename, pt);

            BOOST_FOREACH( ptree::value_type const& v, pt.get_child("deploy") ) {
                if( v.first == "node" ) {
                    node n;
                    n.name = v.second.get<std::string>("<xmlattr>.name");
                    n.address = v.second.get<std::string>("<xmlattr>.address");
                    n.port = v.second.get<port_id>("<xmlattr>.port");
                    nodes.push_back(n);
                    map_node[n.name] = &nodes.back();
                }
                else if( v.first == "map" ) {
                    mapping mapp;
                    mapp.comp_name = v.second.get<std::string>("<xmlattr>.instance");
                    mapp.node_name = v.second.get<std::string>("<xmlattr>.node");
                    mappings.push_back(mapp);
                    map_node_with_components[mapp.node_name].push_back(&mappings.back());
                    map_component_node[mapp.comp_name] = &mappings.back();
                }
                else
                    std::cout << "Unknown tag in the file" << std::endl;
            }
            return true;
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

} // namespace parser
} // namespace protort
} // namespace alpha

#endif // PARSER_H
