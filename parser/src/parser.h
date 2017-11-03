#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace alpha {
namespace protort {
namespace parser {

/*!
* \brief Компонент parser - описывает схему приложения и развертывания,
* получает и хранит информацию путем парсинга xml файлов схемы приложения
* и схемы развертывания.
*
* Стуктуры:
* Компонента, связь между портами, адрес, сетевой узел,
* отображение компонента на сетевой узел, конфигурация.
*/

using boost::property_tree::ptree;
using port_id = uint32_t;

/*!
 * \brief Класс компонент
 *
 * Класс содержит имя и тип компонента
 */

struct component
{
    /*!
     * \brief Имя компонента
     */
    std::string name;

    /*!
     * \brief Тип компонента
     */
    std::string kind;
};

/*!
 * \brief Класс связи между портами компонентов
 *
 * Класс хранит имена компонентов и связанные порты
 */

struct connection
{
    /*!
     * \brief // Идентификатор компонента, отправляющего сообщение
     */
    std::string source;

    /*!
     * \brief Идентификатор выхода у отправляющего компонент
     */
    port_id source_out;

    /*!
     * \brief Идентификатор компонента, получающего сообщение
     */
    std::string dest;

    /*!
     * \brief Идентификатор входа у получающего компонента
     */
    port_id dest_in;
};

/*!
 * \brief Класс адрес
 *
 * Класс хранит имя, адрес, порт узла и конфигурационный порт узла
 */

struct address
{
    /*!
     * \brief IP-адрес или hostname узла
     */
    std::string ip_address;

    /*!
     * \brief Порт
     */
    port_id port;

    /*!
     * \brief Порт сервера конфигурации (по умолчанию =100)
     */
    port_id config_port;
};

/*!
 * \brief Класс сетевого узла
 *
 * Класс хранит имя, адрес ноды и адрес резервной ноды
 */

struct node
{
    /*!
     * \brief Имя
     */
    std::string name;

    /*!
     * \brief Адрес ноды(включает ip адрес, порт и конфигурационный порт)
     */
    address address_;

    /*!
     * \brief Адрес резервной ноды
     */
    boost::optional<address> pairnode;
};

/*!
 * \brief Класс отображения компонента на сетевой узел
 *
 * Класс хранит имя компонента и имя узла
 */

struct mapping
{
    /*!
     * \brief Имя компонента
     */
    std::string comp_name;

    /*!
     * \brief Имя узла
     */
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
    /*!
     * \brief Векторы для хранения соответствующих компонентов после парсинга приложения
     */
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
     * Содержит обработчик ошибок, выводит сообщения об ошибках в стандартный поток вывода.
     */

     bool parse_app(const std::string &filename)
     {
         try
         {
            ptree pt;// Cоздаем дерево
            read_xml(filename, pt);// Парсим xml в дерево pt

            BOOST_FOREACH(ptree::value_type const &v, pt.get_child("app") ) {// Создаем компоненты для тэгов app .
            if( v.first == "instance" )
                parse_component(v);
            else if( v.first == "connection" )
                parse_connection(v);
            else
                throw std::invalid_argument((boost::format("Unknown tag in the file %0%.") % filename ).str());
            }
            return true;
         }

         catch(std::exception &error)
         {
             std::cout << "Error: " << error.what() << std::endl;
             return false;
         }
     }

     /*!
      * \brief Парсит схему конфигураций
      * \param путь к файлу, в котором находится схема приложения в формате xml
      * \return true если успешно, иначе false
      *
      * Метод парсит схему конфигураций и хранит полученную информацию внутри класса в соответствующих атрибутах
      * Содержит обработчик ошибок,выводит сообщения об ошибках в стандартный поток вывода.
      */

     bool parse_deploy(const std::string &filename)
     {
         try
         {
             ptree pt;// Cоздаем дерево
             read_xml(filename, pt);// Парсим xml в дерево pt

             BOOST_FOREACH(ptree::value_type const &v, pt.get_child("deploy") ) {// Создаем компоненты для тегов deploy
                 if( v.first == "node")
                     parse_node(v);
                 else if( v.first == "map" )
                     parse_map(v);
                 else
                     throw std::invalid_argument((boost::format("Unknown tag in the file %0%.") % filename ).str());
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

     void parse_node(ptree::value_type const &v)
     {
         /*!
          * \brief Количество детей у поддерева node
          */
         auto child_size = v.second.size();

         if(child_size > 2)
             throw std::invalid_argument((boost::format("ERROR in the node name (%0%): Expected no more than one child node: 'pairnode'")
                                          % v.second.get<std::string>("<xmlattr>.name")).str());
         else if(child_size == 0)
                 throw std::invalid_argument("Incorrectly entered node.");
         else{
             node current_node;

             /*!
              * \brief Инициализируем поля узла, исходя из описания в xml
              */
             current_node.name = v.second.get<std::string>("<xmlattr>.name");
             current_node.address_.ip_address = v.second.get<std::string>("<xmlattr>.address");
             current_node.address_.port = v.second.get<port_id>("<xmlattr>.port");
             current_node.address_.config_port = v.second.get<port_id>("<xmlattr>.config_port", 100);

             if(child_size == 2){
                 address pair_address;

                 /*!
                  * \brief Инициализируем поля резервного узла, исходя из описания в xml
                  */
                 pair_address.ip_address= v.second.get<std::string>("pairnode.<xmlattr>.address");
                 pair_address.port = v.second.get<port_id>("pairnode.<xmlattr>.port");
                 pair_address.config_port = v.second.get<port_id>("pairnode.<xmlattr>.config_port", 100);
                 current_node.pairnode = pair_address;
             }

             nodes.push_back(current_node);
         }
     }


     /*!
      * \brief Парсит component
      *
     * Метод парсит component и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_component(ptree::value_type const &v)
     {
         component comp;

         /*!
          * \brief Инициализируем поля component, исходя из описания в xml
          */
         comp.name = v.second.get<std::string>("<xmlattr>.name");
         comp.kind = v.second.get<std::string>("<xmlattr>.kind");
         components.push_back(comp);
     }

     /*!
      * \brief Парсит connection
      *
     * Метод парсит connection и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_connection(ptree::value_type const &v)
     {
         connection conn;

         /*!
          * \brief Инициализируем поля connection, исходя из описания в xml
          */
         conn.source = v.second.get<std::string>("<xmlattr>.source");
         conn.source_out = v.second.get<port_id>("<xmlattr>.source_out");
         conn.dest = v.second.get<std::string>("<xmlattr>.dest");
         conn.dest_in = v.second.get<port_id>("<xmlattr>.dest_in");
         connections.push_back(conn);
     }

     /*!
      * \brief Парсит map
      *
     * Метод парсит map и сохраняет полученную информацию внутри класса в соответствующих атрибутах
      */

     void parse_map(ptree::value_type const &v)
     {
         mapping mapp;

         /*!
          * \brief Инициализируем поля mapping, исходя из описания в xml
          */
         mapp.comp_name = v.second.get<std::string>("<xmlattr>.instance");
         mapp.node_name = v.second.get<std::string>("<xmlattr>.node");
         mappings.push_back(mapp);
     }
};

} // namespace parser
} // namespace protort
} // namespace alpha

#endif // PARSER_H
