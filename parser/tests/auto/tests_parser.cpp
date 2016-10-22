#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "parser.h"
namespace alpha {
namespace protort {
namespace parser {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_parser)

BOOST_AUTO_TEST_CASE(test_parser_parse)
{
    //исходные данные
    std::string inst_name = "A";
    std::string inst_kind = "generator";
    std::string con_src = "A";
    short src_out = 1;
    std::string dest = "B";
    short dest_in = 1;

    //сравнение исходных данных и распарсенных
    Deploy_scheme ds;
    ds.parse_app("../testdata/app.xml");
    ds.parse_deploy("../testdata/deploy.xml");
    component comp = ds.get_component(inst_name);
    BOOST_CHECK_EQUAL(comp.name, inst_name);
    BOOST_CHECK_EQUAL(comp.type, inst_kind);
    std::pair<std::string, short> dest_name_port = ds.get_dest_and_port(std::pair<std::string, short>(con_src, src_out));
    BOOST_CHECK_EQUAL(dest_name_port.first, dest);
    BOOST_CHECK_EQUAL(dest_name_port.second, dest_in);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace parser
} // namespace protort
} // namespace alpha
