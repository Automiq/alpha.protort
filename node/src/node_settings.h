#ifndef NODE_SETTINGS_H
#define NODE_SETTINGS_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
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
    uint16_t configuration_port;

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
     * выводится ошибка стандартный поток вывода ошибок. В случае обнаружения
     * параметра --help/-h приложение закрывается со статусом EXIT_SUCCESS
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
            boost::program_options::options_description desc{ "Node Options" };
            desc.add_options()
                    ("help,h", "Help screen")
                    ("config-port,c", boost::program_options::value<uint16_t>(&configuration_port)->default_value(100), "configuration port");


            boost::program_options::variables_map vm;
            boost::program_options::store(parse_command_line(argc, argv, desc), vm);
            boost::program_options::notify(vm);

            if (vm.count("help"))
            {
                std::cout << desc << '\n';
                exit(EXIT_SUCCESS);
            }

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
