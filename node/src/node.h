#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
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
        :client_(*this,service_),
         server_(*this,service_),
         settings_(settings),
         signals_(service_,SIGINT,SIGTERM)
    {

    }

    void start()
    {
        auto start_time = boost::chrono::steady_clock::now();
        switch (settings_.component_kind)
        {
            case alpha::protort::protocol::Packet::Terminator:
            {
                signals_.async_wait(boost::bind(&io_service::stop,&service_));
                server_.listen(settings_.source);
                break;
            }
            case alpha::protort::protocol::Packet::Generator:
            {
                client_.async_connect(settings_.destination);
                break;
            }
        }
        service_.run();
    }

    void on_packet_sent(const boost::system::error_code & err, size_t bytes)
    {

    }

    void on_connected(const boost::system::error_code & err)
    {

    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {

    }

    void on_new_connection(const boost::system::error_code & err)
    {

    }

private:
    io_service service_;
    server<node> server_;
    client<node> client_;
    node_settings settings_;
    signal_set signals_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
