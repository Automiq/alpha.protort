#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "link_server.h"
#include "link_client.h"

namespace alpha {
namespace protort {
namespace link {
namespace tests {

using namespace boost::asio;

const std::string str = "123456789";

struct LinkClientCallbackTest
{
    LinkClientCallbackTest(){}
    void on_connected(link_client<LinkClientCallbackTest>* client)
    {
        std::cout << "Connected\n";
        client->send_async(str);
    }
};
struct LinkServerCallbackTest
{
    LinkServerCallbackTest(){}
    void on_new_connection()
    {
        std::cout << "New client has been connected\n";
    }
    void on_new_packet(char const *buffer, size_t nbytes)
    {
        std::cout << "on_new_packet\n";
    }
};

BOOST_AUTO_TEST_SUITE(tests_link)

BOOST_AUTO_TEST_CASE(test_link)
{
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 100);
    LinkClientCallbackTest callbackclient;
    LinkServerCallbackTest callbackserver;
    link_client<LinkClientCallbackTest> linkclient(callbackclient, service);
    link_server<LinkServerCallbackTest> linkserver(callbackserver,service);

    linkserver.listen();
    linkclient.connect_async(ep);
    //linkclient.send_async(str);
    service.run();
    std::cin.get();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace link
} // namespace protort
} // namespace alpha
