#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/program_options.hpp>
#include "node_settings.h"

namespace alpha {
namespace protort {
namespace node {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_node_settings)

BOOST_AUTO_TEST_CASE(test_node_setiings_parse)
{
    alpha::protort::node::node_settings ns;
    const char p1[] = {'-', '-', 's', '\0'};
    const char p2[] = {'1', '9', '2', '.', '1', '6', '8', '.', '1', '.', '1', ':', '9', '9', '9', '\0'};
    const  char * pp[] = {p1, p2};
    BOOST_CHECK(ns.parse(3, pp));

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
