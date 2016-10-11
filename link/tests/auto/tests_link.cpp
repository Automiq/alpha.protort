#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "link_client.h"

namespace alpha {
namespace protort {
namespace link {
namespace tests {

using namespace boost::asio;

struct LinkClientCallbackTest
{
    LinkClientCallbackTest(){}
    void on_connected()
    {
        std::cout << "Connected\n";
    }
};


BOOST_AUTO_TEST_SUITE(tests_link)

BOOST_AUTO_TEST_CASE(test_link)
{
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("212.73.124.11"), 80);
    LinkClientCallbackTest callbackclient;
    const std::string str = "message";

    link_client<LinkClientCallbackTest> linkclient(callbackclient, service);
    linkclient.connect_async(ep);
    linkclient.send_async(str);
    std::cin.get();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace link
} // namespace protort
} // namespace alpha
