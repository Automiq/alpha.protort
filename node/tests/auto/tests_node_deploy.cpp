#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "node.h"
#include "components.h"
#include "parser.h"

namespace alpha {
namespace protort {
namespace node {
namespace tests {

struct fixture
{
    ~fixture()
    {
        google::protobuf::ShutdownProtobufLibrary();
    }
};

BOOST_FIXTURE_TEST_SUITE(tests_node_deploy,fixture)

BOOST_AUTO_TEST_CASE(test_node_deploy)
{
    alpha::protort::node::node_settings node_settings_;
    const char *argv[] = { "name_of_exe", "--name", "current_node" };
    node_settings_.parse(3, argv);

    node n(node_settings_);

    alpha::protort::parser::configuration cnfg;
    cnfg.parse_app("../tests/auto/testdata/app_test.xml");
    cnfg.parse_deploy("../tests/auto/testdata/deploy_test.xml");

    n.deploy(cnfg);
    n.router_.route("A", 0, "smth");
}
BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
