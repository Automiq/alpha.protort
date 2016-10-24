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

struct node_settings
{
    boost::asio::ip::tcp::endpoint source;
    boost::asio::ip::tcp::endpoint destination;
    alpha::protort::protocol::Packet::ComponentKind component_kind;
    uint32_t packet_size = 0;
    uint32_t npackets = 0;

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
                    ("node-kind,n", boost::program_options::value<std::string>(&node_kind), "node-kind generator|retranslator|terminator")
                    ("packet-size", boost::program_options::value<uint32_t>(&packet_size), "packet size")
                    ("npackets", boost::program_options::value<uint32_t>(&npackets), "number of packet");


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
