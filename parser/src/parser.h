#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

namespace alpha {
namespace protort {
namespace parser {

/*!
 * \brief Класс компонент
 *
 * Класс содержит имя и тип компонента
 */
struct component
{
    std::string name;
    std::string type;
};

/*!
 * \brief Класс связи между портами компонентов
 *
 * Класс хранит имена компонентов и связанные порты
 */
struct connection
{
    std::string source_name;
    unsigned short source_out;
    std::string dest_name;
    unsigned short dest_in;
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
    unsigned short port;
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
            using boost::property_tree::ptree;
            boost::property_tree::ptree pt;
            read_xml(filename, pt);

            BOOST_FOREACH( ptree::value_type const& v, pt.get_child("app") ) {
                if( v.first == "instance" ) {
                    component comp;
                    comp.name = v.second.get<std::string>("<xmlattr>.name");
                    comp.type = v.second.get<std::string>("<xmlattr>.kind");
                    components.push_back(comp);
                }
                else if( v.first == "connection" ) {
                    connection conn;
                    conn.source_name = v.second.get<std::string>("<xmlattr>.source");
                    conn.source_out = v.second.get<unsigned short>("<xmlattr>.source_out");
                    conn.dest_name = v.second.get<std::string>("<xmlattr>.dest");
                    conn.dest_in = v.second.get<unsigned short>("<xmlattr>.dest_in");
                    connections.push_back(conn);
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
            using boost::property_tree::ptree;
            boost::property_tree::ptree pt;
            read_xml(filename, pt);

            BOOST_FOREACH( ptree::value_type const& v, pt.get_child("deploy") ) {
                if( v.first == "node" ) {
                    node n;
                    n.name = v.second.get<std::string>("<xmlattr>.name");
                    n.address = v.second.get<std::string>("<xmlattr>.address");
                    n.port = v.second.get<unsigned short>("<xmlattr>.port");
                    nodes.push_back(n);
                }
                else if( v.first == "map" ) {
                    mapping mapp;
                    mapp.comp_name = v.second.get<std::string>("<xmlattr>.instance");
                    mapp.node_name = v.second.get<std::string>("<xmlattr>.node");
                    mappings.push_back(mapp);
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
