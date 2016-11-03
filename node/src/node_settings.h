#ifndef NODE_SETTINGS_H
#define NODE_SETTINGS_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include "packet.pb.h"

namespace alpha {
namespace protort {
namespace node {

/*!
 * \brief Класс настроек узла
 *
 * Данный класс предназначен для хранения настроек узла.
 */
struct node_settings
{
    /*!
     * \brief Адрес для входящих подключений
     *
     * Узел, работающий в режиме сервера (ретранслятор или терминатор), должен
     * использовать этот адрес для прослушивания входящих подключений.
     */
    boost::asio::ip::tcp::endpoint source;

    /*!
     * \brief Адрес для исходящих подключений
     *
     * Узел, работающий в режиме клиента (генератор или ретранслятор), должен
     * использовать этот адрес для исходящего подключения.
     */
    boost::asio::ip::tcp::endpoint destination;
    uint32_t packet_size = 0;
    uint32_t npackets = 0;
    std::string path_app;
    std::string path_deploy;
    std::string name;

    /*!
     * \brief Тип компонента
     *
     * Определяет тип компонента, который работает на узле. То есть режимы, в
     * которых работает узел.
     */
    alpha::protort::protocol::ComponentKind component_kind;

    /*!
     * \brief Парсит настройки узла из массива строк
     *
     * \param argc Количество строк в массиве
     * \param argv Указатель на массив строк
     * \return true в случае успеха, иначе false
     *
     * Данный метод предназначен для разбора массива строковых аргументов,
     * обычно подаваемых через командную строку, и получения удобной для
     * дальнейшего использования структуры с настройками узла.
     * В случае успешного разбора аргументов, метод загружает настройки в
     * экемпляр класса и возвращает true. В случае ошибки возвращается false и
     * выводится ошибка стандартный поток вывода ошибок.
     *
     * Пример использования:
     * \code
     *  int main(int argc, const char *argv[])
     *  {
     *      alpha::protort::node::node_settings settings;
     *
     *      if (!settings.parse(argc, argv));
     *          // ошибка разбора
     *          return 1;
     *
     *      // успех
     *      // дальнейшая работа с распарсенными настройками
     *      ...
     *      return 0;
     *  }
     * \endcode
     */
    bool parse(int argc, const char **argv)
    {
        try
        {
            std::string source_ip_port_str;
            std::string destination_ip_port_str;
            std::string node_kind;

            boost::program_options::options_description desc{ "Node Options" };
            desc.add_options()
                    ("help,h", "Help screen")
                    ("source,s", boost::program_options::value<std::string>(&source_ip_port_str)->default_value("0.0.0.0:31337"), "Source ip:port")
                    ("destination,d", boost::program_options::value<std::string>(&destination_ip_port_str), "Destination host:port")
                    ("node-kind,n", boost::program_options::value<std::string>(&node_kind), "node-kind generator|retranslator|terminator")
                    ("packet-size", boost::program_options::value<uint32_t>(&packet_size), "packet size")
                    ("npackets", boost::program_options::value<uint32_t>(&npackets), "number of packet")
                    ("app", boost::program_options::value<std::string>(&path_app)->default_value("./app.xml"), "path to app.xml")
                    ("deploy", boost::program_options::value<std::string>(&path_deploy)->default_value("./deploy.xml"), "path to deploy.xml")
                    ("name", boost::program_options::value<std::string>(&name)->default_value("current_node"), "node name");


            boost::program_options::variables_map vm;
            boost::program_options::store(parse_command_line(argc, argv, desc), vm);
            boost::program_options::notify(vm);

            if (vm.count("help"))
                std::cout << desc << '\n';

            if (source_ip_port_str.size()) {
                std::string source_ip;
                short source_port;
                split_ip_port(source_ip_port_str, source_ip, source_port);
                source = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(source_ip), source_port);
            }
            if (destination_ip_port_str.size()){
                std::string destination_ip;
                short destination_port;
                split_ip_port(destination_ip_port_str, destination_ip, destination_port);
                destination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(destination_ip), destination_port);
            }

            if (node_kind == "generator")
                component_kind = alpha::protort::protocol::Generator;
            else if (node_kind == "retranslator")
                component_kind = alpha::protort::protocol::Retranslator;
            else if (node_kind == "terminator")
                component_kind = alpha::protort::protocol::Terminator;

            return true;
        }
        catch (const boost::program_options::error &ex)
        {
            std::cerr << ex.what() << '\n';
            return false;
        }
    }

private:

    void split_ip_port(const std::string& s, std::string& ip, short& port)
    {
        std::string port_str;
        std::string::const_iterator iter;
        iter = std::find(s.begin(), s.end(), ':');
        ip = std::string(s.begin(), iter);
        port_str = std::string(iter + 1, s.end());
        port = std::stoi(port_str);
    }
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_SETTINGS_H
