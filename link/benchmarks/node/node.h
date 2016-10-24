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
namespace node {

using namespace alpha::protort::link;

class node
{

public:

    node(const node_settings &settings)
        :client_(*this,service),
         server_(*this,service),
         settings_(settings),
         signals_(service,SIGINT,SIGTERM),
         msg_(settings_.packet_size,'#')
    {

    }

    void start()
    {
        std::cout << "start" << std::endl;
        auto start_time = boost::chrono::steady_clock::now();
        switch (settings_.component_kind)
        {
            case alpha::protort::protocol::Packet::Terminator:
            {
                signals_.async_wait(boost::bind(&io_service::stop,&service));
                server_.listen(settings_.source);
                break;
            }
            case alpha::protort::protocol::Packet::Generator:
            {
                client_.async_connect(settings_.destination);
                break;
            }
        }

        std::cout << "npacket:" << settings_.npackets << std::endl << "size of packet: " << settings_.packet_size << std::endl;
        service.run();
        auto end_time = boost::chrono::steady_clock::now();
        boost::chrono::duration<double> duration_ = end_time - start_time;
        std::cout << "Run time: " << duration_.count() << std::endl;
        std::cout << "Speed: " << (8*settings_.packet_size * settings_.npackets / (1024*1024*1024*duration_.count())) << std::endl;
        std::cout << "Speed (GBit/s): " << (8*double(settings_.packet_size) * settings_.npackets / (1024*1024*1024*duration_.count())) << std::endl;
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
    server<node> server_;
    client<node> client_;
    node_settings settings_;
    signal_set signals_;
    int packet_counter_server = 0;
    int packet_counter_client = 0;
    const std::string msg_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
