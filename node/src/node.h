#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "packet.pb.h"
#include "server.h"
#include "client.h"

namespace alpha {
namespace protort {
namespace node {

using namespace alpha::protort::link;

io_service service;

static const std::string msg(max_packet_size,'#');

class node
{
public:

    node(ip::tcp::endpoint& ep,alpha::protort::protocol::Packet::ComponentKind& component_kind)
        :client_(*this,service),
         server_(*this,service),
         component_kind_(component_kind),
         ep_(ep)
    {

    }

    void start()
    {
        switch (component_kind_)
        {
            case alpha::protort::protocol::Packet::Terminator:
            {
                server_.listen(ep_);
                break;
            }
            case alpha::protort::protocol::Packet::Generator:
            {
                client_.async_connect(ep_);
                break;
            }
        }
        service.run();
    }

    void on_connected()
    {
        for(int i = 0;i < amount_of_packets;i++)
        {
            client_.async_send(msg);
        }
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
#ifdef _DEBUG
        if(++packet_counter == amount_of_packets)
            std::cout << "All packets have been received." << std::endl;
        if(packet_counter % 1000 == 0)
            std::cout << packet_counter << " packets have been received" << std::endl;
#endif
    }

    void on_new_connection()
    {

    }

private:
    server<node> server_;
    client<node> client_;
    alpha::protort::protocol::Packet::ComponentKind& component_kind_;
    ip::tcp::endpoint& ep_;
    int packet_counter = 0;
    const int amount_of_packets = 1000000;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
