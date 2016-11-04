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
    const char* a ("name_of_exe");
    const char* b ("--name");
    const char* c ("node");
    const char* argv[] = { a, b, c };
    BOOST_CHECK(ns.parse(3, argv));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
