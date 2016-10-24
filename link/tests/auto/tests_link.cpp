#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "server.h"
#include "client.h"

namespace alpha {
namespace protort {
namespace link {
namespace tests {

using namespace boost::asio;

const std::string str = "hello_world";
const std::string str1 = "Bis_bald_world";

struct node
{
    client<node> m_client;
    server<node> m_server;
    int counter_ = 0;

    node(io_service& service)
        :m_client(*this,service),
         m_server(*this,service)
    {
#ifdef _DEBUG
        std::cout << "node ctor" << std::endl;
#endif
    }

    void on_connected(const boost::system::error_code & err)
    {
        m_client.async_send(str);
#ifdef _DEBUG
        std::cout << "on_connected" << std::endl;
#endif
    }

    void on_packet_sent(const boost::system::error_code & err, size_t nbytes)
    {
        if(++counter_ < 2)
            m_client.async_send(str1);
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
#ifdef _DEBUG
        std::cout << std::string(buffer,nbytes) << "\n";
        std::cout << "on_new_packet\n";
        std::cout.flush();
#endif
    }

    void on_new_connection(const boost::system::error_code & err)
    {
#ifdef _DEBUG
        std::cout << "New client has been connected" << std::endl;
#endif
    }
};

BOOST_AUTO_TEST_SUITE(tests_link)

BOOST_AUTO_TEST_CASE(test_link)
{
    /*
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("0.0.0.0"), 31337);
    node node_(service);

    node_.m_server.listen(ep);
    node_.m_client.async_connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 31337));

    service.run();
    */
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace link
} // namespace protort
} // namespace alpha
