#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
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
bool operator <(component const& left, component const& right)
{
    return left.name < right.name;
}
/*!
 * \brief Класс сетевого узла
 *
 * Класс хранит имя, адрес и порт узла
 */
struct node
{
    std::string id;
    std::string ip;
    unsigned short port;
};
/*!
 * \brief Класс, описывающий схему приложения и развертывания
 *
 * Данный класс получает и хранит информацию путем парсинга xml файлов схемы приложения
 * и схемы развертывания.
 */
class Deploy_scheme
{
public:
    typedef std::pair<std::string, short> comp_name_port;
    typedef std::pair<comp_name_port, comp_name_port> connection;
    /*!
     * \brief Парсит схему приложения
     * \param путь к файлу схемы приложения
     *
     * Метод парсит схему приложения и хранит полученную информацию внутри класса в соответствующих атрибутах
     * Содержит обработчик ошибок, которые обычно возникают на этапе чтения xml файла, выводит сообщения
     * об ошибках в стандартный вывод.
     */
    bool parse_app(const std::string &filename);
    /*!
     * \brief Парсит схему развертывания
     * \param путь к файлу схемы развертыания
     *
     * Метод парсит схему развертывания и хранит полученную информацию внутри класса в соответствующих атрибутах
     * Содержит обработчик ошибок, которые обычно возникают на этапе чтения xml файла, выводит сообщения
     * об ошибках в стандартный вывод.
     */
    bool parse_deploy(const std::string &filename);
    /*!
     * \brief Находит компонент по его имени
     * \param Имя компонента
     * \return Компонент
     */
    component& get_component(std::string& id)
    {
        return id_to_component[id];
    }
    /*!
     * \brief Находит узел по его имени
     * \param Имя узла
     * \return Узел
     */
    std::string& get_node(std::string comp_name)
    {
        return component_to_node[comp_name];
    }
    /*!
     * \brief Находит имя  принимающего компонента и номер входа по имени отправителя и номера выхода
     * \param Пара <Имя компонента, номер выхода>
     * \return Пара <Имя компонента, номер выхода>
     *
     * В отображении связей ищет имя и вход получателя по имени и выходу отправителя
     */
    comp_name_port get_dest_and_port(comp_name_port source_name_port)
    {
        return connections[source_name_port];
    }
private:
    /*!
     * \brief Отображение имени компонента на сам компонент
     */
    std::map<std::string, component> id_to_component;
    /*!
     * \brief Отображение имени узла на сам узел
     */
    std::map<std::string, node> id_to_node;
    /*!
     * \brief Отображение связей между портами отправителя и получателя
     * Ассоциативный контейнер, где ключом является пара <Имя отправляющего компонента, номер выхода>,
     * а значением является пара <Имя принимающего компонента, номер входа>
     */
    std::map<comp_name_port, comp_name_port> connections;
    /*!
     * \brief Отображение компонета на узел
     * Ассоциативный контейнер, где ключом является имя компонента,
     * а значением является имя узла, на котором этот компонент будет работать
     */
    std::map<std::string, std::string> component_to_node;
};

bool Deploy_scheme::parse_app(const std::string &filename)
{
    try
    {
        // Create an empty property tree object
        using boost::property_tree::ptree;
        boost::property_tree::ptree pt;

        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        read_xml(filename, pt);

        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("app") ) {
            if( v.first == "instance" ) {
                component comp;
                comp.name = v.second.get<std::string>("<xmlattr>.name");
                comp.type = v.second.get<std::string>("<xmlattr>.kind");
                id_to_component.insert(std::pair<std::string, component>(comp.name, comp));
            }
            else if( v.first == "connection" ) {
                std::string source_name = v.second.get<std::string>("<xmlattr>.source");
                short source_out = v.second.get<short>("<xmlattr>.source_out");
                std::string dest_name = v.second.get<std::string>("<xmlattr>.dest");
                short dest_in = v.second.get<short>("<xmlattr>.dest_in");
                connections.insert(connection(comp_name_port(source_name, source_out), comp_name_port(dest_name, dest_in)));
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
bool Deploy_scheme::parse_deploy(const std::string &filename)
{
    try
    {
        // Create an empty property tree object
        using boost::property_tree::ptree;
        boost::property_tree::ptree pt;

        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        read_xml(filename, pt);

        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("deploy") ) {
            if( v.first == "node" ) {
                node n;
                n.id = v.second.get<std::string>("<xmlattr>.name");
                n.ip = v.second.get<std::string>("<xmlattr>.address");
                n.port = v.second.get<unsigned short>("<xmlattr>.port");
                id_to_node.insert(std::pair<std::string, node>(n.id, n));
            }
            else if( v.first == "map" ) {
                std::string comp_name = v.second.get<std::string>("<xmlattr>.instance");
                std::string node_name = v.second.get<std::string>("<xmlattr>.node");
                component_to_node.insert(std::make_pair(comp_name, node_name));
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
bool operator <(std::pair<std::string, short>& left, std::pair<std::string, short>& right)
{
    return left.first < right.first;
}

#endif // PARSER_H
