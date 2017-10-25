#ifndef NODE_SETTINGS_H
#define NODE_SETTINGS_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/thread.hpp>

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
    uint16_t threads;

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
    {// парсер аргументов командной строки
        try
        {
            boost::program_options::options_description desc{ "Node Options" };
            // содаем объек класса options_description. описание опций
            //параметр командной строки анализируется как пара ключ / значение,

            desc.add_options()
                    ("help,h", "Help screen")

                    ("config-port,c", boost::program_options::value<uint16_t>(&configuration_port)
                     ->default_value(100), "configuration port")
                    //конфигурация порта, дефолтное значение 100
                    ("threads,t", boost::program_options::value<uint16_t>(&threads)
                     ->default_value(boost::thread::hardware_concurrency()), "threads");
                       //информация о потоках. подефолту hardware_concurrency() возвращает
                      // количествоаппаратных потоков доступных в текущей системе

            boost::program_options::variables_map vm; //потомок std::map
            // vm контейнер для значений. храться в виде значение - ключ
            boost::program_options::store(parse_command_line(argc, argv, desc), vm);
            //принимает опции парсера и переменную variables_map. сохраняет значения в vm
            boost::program_options::notify(vm); // Найти!

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
