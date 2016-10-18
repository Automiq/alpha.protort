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

void split_ip_port(const std::string& s, std::string& ip, short& port)
{
    std::string port_str;
    std::string::const_iterator iter;
    iter = std::find(s.begin(), s.end(), ':');
    ip = std::string(s.begin(), iter);
    port_str = std::string(iter + 1, s.end());
    port = std::stoi(port_str);
}

/**
 * \brief Настройки endpoint для сервера и клиента, а так же Component Kind пакета,  вводимые через командную строку
 *
 * Настройки Node. Для создания настроек доступны следующие опции, вводимые через командную строку при запуске программы:
 * \n -h [ --help ]                                        Вывод доступных команд
 * \n -s [ --source ] arg ( по умолчанию  0.0.0.0:31337)   Source ip:port
 * \n -d [ --destination ] arg                             Destination host:port
 * \n -n [ --node-kind ] arg                               node-kind generator|retranslator|terminator
 * */

struct node_settings
{
    boost::asio::ip::tcp::endpoint source;                              ///< эндпоинт, который будет прослушиваться сервером
    boost::asio::ip::tcp::endpoint destination;                         ///< эндпоинт, используемый клиентом для подключения к серверу
    alpha::protort::protocol::Packet::ComponentKind component_kind;     ///< тип компонента, который будет передаваться в пакете


    /** Для работы необходимо в main(int argc, const char **argv) создать экземпляр node_settings и вызвать его метод node_settings::parse(argc, argv), например:
 * \code
 * int main(int argc, const char *argv[])
 *{
 *   alpha::protort::node::node_settings node_settings;
 *   node_settings.parse(argc, argv);
 *  ...
 * \endcode
 * Введенные настройки хранятся и доступны как атрибуты node_settings::"attribute_name":
 * \n boost::asio::ip::tcp::endpoint source;
 * \n boost::asio::ip::tcp::endpoint destination;
 * \n alpha::protort::protocol::Packet::ComponentKind component_kind;
 * \param argc Количество аргументов, переданных функции main()
 * \param argv Массив строк, переданных функции main()
 * \return True, если не было ошибки при выполнении функции, иначе False
 * */
    bool parse(int argc, const char **argv)
    {
        try
        {
            std::string source_ip_port_str;
            std::string destination_ip_port_str;
            std::string node_kind;

            boost::program_options::options_description desc{ "Node Setup Options" };
            desc.add_options()
                    ("help,h", "Help screen")
                    ("source,s", boost::program_options::value<std::string>(&source_ip_port_str)->default_value("0.0.0.0:31337"), "Source ip:port")
                    ("destination,d", boost::program_options::value<std::string>(&destination_ip_port_str), "Destination host:port")
                    ("node-kind,n", boost::program_options::value<std::string>(&node_kind), "node-kind generator|retranslator|terminator");

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
                component_kind = alpha::protort::protocol::Packet::Generator;
            else if (node_kind == "retranslator")
                component_kind = alpha::protort::protocol::Packet::Retranslator;
            else if (node_kind == "terminator")
                component_kind = alpha::protort::protocol::Packet::Terminator;

            return true;
        }
        catch (const boost::program_options::error &ex)
        {
            std::cerr << ex.what() << '\n';
            return false;
        }
    }
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_SETTINGS_H
