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
* \brief Компонент parser - описывает схему приложения и развертывания,
* получает и хрании информацию путем парсинга xml файлов схемы приложения
* и схемы развертывания.
*
* Стуктуры:
* Компонента, связь между портами, сетевой узел, резервный узел
* отображение компонента на сетевой узел, конфигурация.
*/

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
    std::string source;// Идентификатор компонента, отправляющего сообщение
    port_id source_out;// Идентификатор выхода у отправляющего компонента
    std::string dest;// Идентификатор компонента, получающего сообщение
    port_id dest_in;// Идентификатор входа у получающего компонента
};

/*!
 * \brief Класс сетевого узла
 *
 * Класс хранит имя, адрес, порт узла и конфигурационный порт узла
 */

struct Address
{
    std::string address;// IP-адрес или hostname узла
    port_id port;// Порт
    port_id config_port;// Порт сервера конфигурации (по умолчанию =100)
};

struct node
{
    std::string name;// Имя
    Address address;// Адрес ноды(включает ip адрес, порт и конфигурационный порт)
    boost::optional<Address> pairnode; // Адрес резервной ноды
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

struct configuration:std::exception
{
    //Векторы для хранения соответствующих компонентов после парсинга приложения
    std::vector<component> components;
    std::vector<connection> connections;
    std::vector<node> nodes;
    std::vector<mapping> mappings;

    /*!
     * \brief Парсит схему приложения
     * \param путь к файлу, в котором находится схема приложения в формате xml
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
            boost::property_tree::ptree pt;// Cоздаем дерево
            read_xml(filename, pt);// Парсим xml в дерево pt

            BOOST_FOREACH( boost::property_tree::ptree::value_type const &v, pt.get_child("app") ) {// Создаем компоненты для тэгов app .
            if( v.first == "instance" ) {
                parse_component(v);
            }
            else if( v.first == "connection" ) {
                parse_connection(v);
            }
            else
                std::cout << "Unknown tag in the file" << std::endl;
            }
            return true;
         }

         catch(std::exception &error)
         {
             std::cout << "Error: " << error.what() << std::endl;
             return false;
         }
     }


     bool parse_deploy(const std::string &filename)
     {
         try
         {
             boost::property_tree::ptree pt;// Cоздаем дерево
             read_xml(filename, pt);// Парсим xml в дерево pt

             BOOST_FOREACH( boost::property_tree::ptree::value_type const &v, pt.get_child("deploy") ) {// Создаем компоненты для тегов deploy
                 if( v.first == "node") {

                     auto size = v.second.size();

                     if(size > 2)
                     {
                         throw std::invalid_argument("The node name("
                                                     + v.second.get<std::string>("<xmlattr>.name")
                                                     + ") has more than one pairnode");
                     }
                     else{
                         if(size == 0)
                             throw std::exception::exception("Incorrectly entered node!");
                         else{
                             node n;
                             parse_node(v, n);

                             if(size == 2)
                                parse_node(v, n, true);
                         }
                     }
                 }
                 else if( v.first == "map" )
                     parse_map(v);
                 else
                     std::cout << "Unknown tag in the file" << std::endl;
             }
             return true;
         }

         catch(std::exception &error)
         {
             std::cout << "Error: " << error.what() << std::endl;
             return false;
         }
     }

private:

     /*!
      * \brief Парсит node
      *
     * Метод парсит node и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_node(boost::property_tree::ptree::value_type const &v, node &n, bool flag = false)
     {
         n.name = v.second.get<std::string>("<xmlattr>.name");// Устанавливаем  имя узла, соответстующий иадресу из xml кода
         n.address.address = v.second.get<std::string>("<xmlattr>.address");// Устанавливаем  адресс узла, соответстующий иадресу из xml кода
         n.address.port = v.second.get<port_id>("<xmlattr>.port");// Устанавливаем  порт узла, соответстующий порту из xml кода
         n.address.config_port = v.second.get<port_id>("<xmlattr>.config_port", 100);//Устанавливаем  конфигурацию, соответстующую конфигурации из xml кода

         if(flag){
            Address tmp;
            tmp.pairnode.= v.second.get<std::string>("pairnode.<xmlattr>.address");// Устанавливаем  адресс узла, соответстующий иадресу из xml кода
            tmp.port = v.second.get<port_id>("pairnode.<xmlattr>.port");// Устанавливаем  порт узла, соответстующий порту из xml кода
            tmp.config_port = v.second.get<port_id>("pairnode.<xmlattr>.config_port", 100);//Устанавливаем  конфигурацию, соответстующую конфигурации из xml кода
            n.pairnode = tmp;
         }
         nodes.push_back(n);// Пушим узел в вектор узлов
     }

     /*!
      * \brief Парсит component
      *
     * Метод парсит connection и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_component(boost::property_tree::ptree::value_type const &v)
     {
         component comp;
         comp.name = v.second.get<std::string>("<xmlattr>.name");// Задаем имя компонента, соответстующее имени из xml кода
         comp.kind = v.second.get<std::string>("<xmlattr>.kind");// Задаем тип компонента, соответстующий типу из xml кода
         components.push_back(comp);// Пушим компонент в вектор компонентов
     }

     /*!
      * \brief Парсит connection
      *
     * Метод парсит connection и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_connection(boost::property_tree::ptree::value_type const &v)
     {
         connection conn;
         conn.source = v.second.get<std::string>("<xmlattr>.source");// Задаем идентификатор компонента у отправляющего компонента
         conn.source_out = v.second.get<port_id>("<xmlattr>.source_out");// Задаем идентификатор выхода у отправляющего компонента
         conn.dest = v.second.get<std::string>("<xmlattr>.dest");// Задаем идентификатор компонента, получающего сообщение
         conn.dest_in = v.second.get<port_id>("<xmlattr>.dest_in");//Задаем идентификатор входа у получающего компонента
         connections.push_back(conn);//Пушим связь в вектор связей
     }

     /*!
      * \brief Парсит map
      *
     * Метод парсит map и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_map(boost::property_tree::ptree::value_type const &v)
     {
         mapping mapp;
         mapp.comp_name = v.second.get<std::string>("<xmlattr>.instance");// Устанавливаем имя компонента
         mapp.node_name = v.second.get<std::string>("<xmlattr>.node");// Устанавливаем имя узла
         mappings.push_back(mapp);// Пушим отображение компонента на сетевой узел в соответствующий вектор
     }
};

} // namespace parser
} // namespace protort
} // namespace alpha

#endif // PARSER_H
