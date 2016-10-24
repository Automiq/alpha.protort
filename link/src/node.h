#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/timer.hpp>

#include "client.h"
#include "server.h"

#include "node_settings.h"

#include "packet.pb.h"

namespace alpha {
namespace protort {
namespace node {

using alpha::protort::link::client;
using alpha::protort::link::server;
using alpha::protort::protocol::ComponentKind;

class node
{
public:
    node(node_settings const& settings)
        : settings_(settings),
          client_(*this, io_service_),
          server_(*this, io_service_)
    {
    }

    ~node()
    {
        std::cout << "packets sent " << packets_sent_ << " received " << packets_received_ << std::endl;
    }

    void start()
    {
        boost::timer timer;

        switch (settings_.component_kind) {
        case ComponentKind::Generator:
            client_.async_connect(settings_.destination);
            break;
        case ComponentKind::Terminator:
            server_.listen(settings_.source);
            break;
        case ComponentKind::Retranslator:
            client_.async_connect(settings_.destination);
            server_.listen(settings_.source);
            break;
        default:
            assert(false);
            break;
        }

        boost::asio::signal_set signals(io_service_, SIGINT, SIGTERM);

        if (settings_.component_kind != ComponentKind::Generator)
        {
            signals.async_wait(
                boost::bind(&boost::asio::io_service::stop, &io_service_));
        }

        io_service_.run();

        auto seconds = timer.elapsed();
        uint64_t total_bytes = uint64_t(settings_.n_packets) * uint64_t(settings_.packet_size);
        auto throughput = total_bytes / seconds;
        auto throughput_mib = throughput / 1024 / 1024;

        std::cout << "total packets " << settings_.n_packets << std::endl;
        std::cout << "bytes per packet " << settings_.packet_size << std::endl;
        std::cout << "total bytes " << total_bytes << std::endl;
        std::cout << "seconds " << seconds << std::endl;
        //std::cout << "throughput " << throughput << " bytes/s" << std::endl;
        std::cout << "throughput " << throughput_mib << " MiB/s ("
                  << throughput_mib / 1024 << " GiB/s)" << std::endl;
        std::cout << "throughput " << throughput_mib * 8 << " MBit/s ("
                  << (throughput_mib / 1024) * 8 << " GBit/s)" << std::endl;
    }

    void do_send_packet()
    {
        static const std::string payload(settings_.packet_size, '#');
        client_.async_send(payload);
    }

    void on_connected(const boost::system::error_code & err)
    {
#ifdef _DEBUG
        std::cout << "on_connected" << std::endl;
#endif

        std::cout << "sending " << settings_.n_packets << " packets "
                  << settings_.packet_size << " bytes long each..." << std::endl;

//        static const std::string payload(settings_.packet_size, '#');

//        for (auto n = 0; n < settings_.n_packets; ++n)
//            client_.async_send(payload);

        do_send_packet();
    }

    void on_packet_sent(const boost::system::error_code & err, size_t bytes)
    {
#ifdef _DEBUG
        std::cout << "on_packet_sent err=" << err.message() << " bytes=" << bytes << std::endl;
#endif
        if (++packets_sent_ < settings_.n_packets)
            do_send_packet();
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
#ifdef _DEBUG
        std::cout << std::string(buffer,nbytes) << "\n";
        std::cout << "on_new_packet\n";
        std::cout.flush();
#endif
        ++packets_received_;

        //if (packets_received_ == settings_.n_packets)
        //    std::cout << "received all packets";
    }

    void on_new_connection()
    {
#ifdef _DEBUG
        std::cout << "New client has been connected" << std::endl;
#endif
    }

private:
    io_service io_service_;
    node_settings settings_;
    client<node> client_;
    server<node> server_;

    uint32_t packets_sent_;
    uint32_t packets_received_;


    //static const uint32_t npackets = 10000;

};

} //namespace node
} //namespace protort
} //namespace alpha

#endif // NODE_H
