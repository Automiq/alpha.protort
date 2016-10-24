#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <iostream>

#include "server.h"
#include "client.h"
#include "node_settings.h"

namespace alpha {
namespace protort {
namespace link {
namespace benchmarks {

using namespace alpha::protort::link;

/*!
 * \brief Тестовый сетевой узел
 *
 * На данном бенчмарке производились первые замеры KPI сетевого стека по
 * схеме generator -> terminator.
 * В тестах использовалось одно ядро процессора, отправлялись пакеты размером
 * 200 KiB в количестве 10k штук.
 * Полученные результаты:
 *  - Intel Core i7-2600, loopback, Windows 10, 3.83 секунды => 3.5-4 GBit/s
 *  - Intel Core i7-2600, loopback, Ubuntu Xenial Xerus 16.04, => ~12 GBit/s
 *    (не ясно откуда такой разброс по сравнению с Windows)
 *  - Intel Celeron CPU B830 @ 1.80 GHz, loopback, Windows 10, 6.71 секунды =>
 *    ~2.22 GBit/s
 */
class test_node
{
public:
    test_node(const node_settings &settings)
        : client_(*this, service),
          server_(*this, service),
          settings_(settings),
          signals_(service, SIGINT, SIGTERM),
          msg_(settings_.packet_size, '#')
    {
    }

    void start()
    {
        auto start_time = boost::chrono::steady_clock::now();

        switch (settings_.component_kind)
        {
        case alpha::protort::protocol::Terminator:
            signals_.async_wait(boost::bind(&io_service::stop,&service));
            server_.listen(settings_.source);
            break;
        case alpha::protort::protocol::Generator:
            client_.async_connect(settings_.destination);
            break;
        default:
            assert(false);
            break;
        }

        std::cout << "npacket:" << settings_.npackets << std::endl << "size of packet: " << settings_.packet_size << std::endl;
        service.run();

        boost::chrono::duration<double> duration_ = boost::chrono::steady_clock::now() - start_time;
        std::cout << "Run time: " << duration_.count() << std::endl;
        std::cout << "Speed: " << (8*double(settings_.packet_size) * settings_.npackets / (1024*1024*1024*duration_.count())) << " GBit/s" << std::endl;
    }

    void on_packet_sent(const boost::system::error_code & err, size_t bytes)
    {
        if(++packet_counter_client < settings_.npackets)
            client_.async_send(msg_);
    }

    void on_connected(const boost::system::error_code & err)
    {
        client_.async_send(msg_);
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
        if(++packet_counter_server == settings_.npackets)
            std::cout << "All packets have been received." << std::endl;
    }

    void on_new_connection(const boost::system::error_code & err)
    {
    }

private:
    io_service service;
    server<test_node> server_;
    client<test_node> client_;
    node_settings settings_;
    signal_set signals_;
    int packet_counter_server = 0;
    int packet_counter_client = 0;
    const std::string msg_;
};

} // namespace benchmarks
} // namespace link
} // namespace protort
} // namespace alpha

#endif // NODE_H
