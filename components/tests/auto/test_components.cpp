#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "node.h"

namespace alpha {
namespace protort {
namespace components {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_components)

BOOST_AUTO_TEST_CASE(test_generator)
{
//    boost::asio::io_service service;
//    node::router<node::node> r(service);
//    alpha::protort::components::generator gen(r);
//    alpha::protort::components::output_list result = gen.process(1, "empty");
//    BOOST_CHECK(!result[0].payload.empty());
//    BOOST_CHECK_EQUAL(0, result[0].ports[0]);
}



BOOST_AUTO_TEST_CASE(test_retranslator)
{
    boost::asio::io_service service;
    node::router<node::node> r(service);
    alpha::protort::components::retranslator retr(r);
    retr.do_process(3, "smth");
    BOOST_CHECK_EQUAL(2, retr.in_port_count());
    BOOST_CHECK_EQUAL(1, retr.in_packet_count());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace components
} // namespace protort
} // namespace alpha
