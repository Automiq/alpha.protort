#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "generator.h"
#include "i_component.h"
#include "retranslator.h"

namespace alpha {
namespace protort {
namespace components {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_components)

BOOST_AUTO_TEST_CASE(test_generator)
{
    alpha::protort::components::generator gen;
    alpha::protort::components::output_list result = gen.process(1, "empty");    
    BOOST_CHECK(!result[0].payload.empty());
    BOOST_CHECK_EQUAL(0, result[0].ports[0]);    
}

BOOST_AUTO_TEST_CASE(test_retranslator)
{
    alpha::protort::components::retranslator retr;
    alpha::protort::components::output_list result = retr.process(1, "smth");
    BOOST_CHECK_EQUAL("smth", result[0].payload);
    BOOST_CHECK_EQUAL(0, result[0].ports[0]);
}
BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace components
} // namespace protort
} // namespace alpha
