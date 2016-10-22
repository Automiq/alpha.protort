#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "packet.pb.h"
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
         sig_term(service,SIGINT,SIGTERM)
    {

    }

    void start()
    {
        sig_term.async_wait(boost::bind(&io_service::stop,&service));

        switch (settings_.component_kind)
        {
            case alpha::protort::protocol::Packet::Terminator:
            {
                server_.listen(settings_.source);
//#ifdef _DEBUG
                std::cout << "Started listenig" << std::endl;
//#endif
                break;
            }
            case alpha::protort::protocol::Packet::Generator:
            {
                client_.async_connect(settings_.destination);
//#ifdef _DEBUG
                std::cout << "Started connecting" << std::endl;
//#endif

                break;
            }
        }

        std::cout << "npacket:" << settings_.npackets << std::endl << "size of packet: " << settings_.packet_size << std::endl;
        service.run();

        std::cout << "Server stoped" << std::endl;
    }

    void on_connected()
    {
        static const std::string msg(settings_.packet_size,'#');
        for(int i = 0; i < settings_.npackets; ++i)
        {
            std::cout << i << std::endl;
            client_.async_send(msg);
        }
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
        std::cout << "New packet !!!" << packet_counter + 1 << std::endl;
//#ifdef _DEBUG
        if(++packet_counter == settings_.npackets)
            std::cout << "All packets have been received." << std::endl;
        if(packet_counter % 1000 == 0)
            std::cout << packet_counter << " packets have been received" << std::endl;
//#endif
    }

    void on_new_connection()
    {

    }

private:
    io_service service;
    server<node> server_;
    client<node> client_;
    node_settings settings_;
    signal_set sig_term;
    int packet_counter = 0;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
