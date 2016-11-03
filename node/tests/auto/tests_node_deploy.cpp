#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#define private public

#include "node.h"
#include "components.h"
#include "node_deploy.h"
#include "../../parser/src/parser.h"

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
    node_settings node_settings_;
    const char *argv[] = { "--s", "192.168.1.1:999" };

    node n(node_settings_);

    alpha::protort::parser::configuration cnfg;
    cnfg.parse_app("../tests/auto/testdata/app_test.xml");
    cnfg.parse_deploy("../tests/auto/testdata/deploy_test.xml");

    node_deploy node_deploy_;
    node_deploy_.get_node_config(cnfg);
    node_deploy_.deploy(n);
    n.router_.route("A", 0, "smth");
}
BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
