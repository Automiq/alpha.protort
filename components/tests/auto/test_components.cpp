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
    boost::asio::io_service service;
    node::router<node::node> r(service);
    alpha::protort::components::generator gen(r);
    alpha::protort::components::output_list result = gen.process(1, "empty");    
    BOOST_CHECK(!result[0].payload.empty());
    BOOST_CHECK_EQUAL(0, result[0].ports[0]);    
}

BOOST_AUTO_TEST_CASE(test_retranslator)
{
    boost::asio::io_service service;
    node::router<node::node> r(service);
    alpha::protort::components::retranslator retr(r);
    alpha::protort::components::output_list result = retr.process(1, "smth");
    BOOST_CHECK_EQUAL("smth", result[0].payload);
    BOOST_CHECK_EQUAL(0, result[0].ports[0]);
}

BOOST_AUTO_TEST_CASE(test_reduce)
{
    using namespace alpha::protort::components;

    output_list out;

    max imax;
    imax.do_process(0,"1.0");
    imax.do_process(1,"2.0");
    out = imax.do_process(2,"3.0");
    BOOST_CHECK_EQUAL(out[0].payload,"3.000000");

    out.clear();
    min imin;
    imin.do_process(0,"1.0");
    imin.do_process(1,"2.0");
    out = imin.do_process(2,"3.0");
    BOOST_CHECK_EQUAL(out[0].payload,"1.000000");

    out.clear();
    average iav;
    iav.do_process(0,"1.0");
    iav.do_process(1,"2.0");
    out = iav.do_process(2,"3.0");
    BOOST_CHECK_EQUAL(out[0].payload,"2.000000");

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace components
} // namespace protort
} // namespace alpha
