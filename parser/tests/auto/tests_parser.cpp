#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <string>
#include "parser.h"

/*  костыли для получения пути к исполняемому файлу ----------
#ifdef WIN32
#include <windows.h>
std::wstring Location()
{
    wchar_t buff[MAX_PATH+10]={0};

    GetModuleFileNameW(NULL, buff, MAX_PATH);

    std::wstring fulllocation(buff);

    int last_slash = fulllocation.find_last_of(L"\\");
    fulllocation = fulllocation.substr(0, last_slash);

    return fulllocation;
}
#else
#include <limits.h>
#include <unistd.h>
char result[ PATH_MAX ];
ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
std::string path( result, (count > 0) ? count : 0 );

#endif
---------------------------------------------------------------*/

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

/*  костыли для получения пути к исполняемому файлу ------------
#ifdef WIN32 //#if (BOOST_OS_WINDOWS)
    std::wstring path = Location();
    std::wstring app_path_w = path + L"\\..\\testdata\\app.xml";
    std::string app_path( app_path_w.begin(), app_path_w.end() );
    std::wstring deploy_path_w = path + L"\\..\\testdata\\deploy.xml";
    std::string deploy_path( deploy_path_w.begin(), deploy_path_w.end() );

#else //#elif (BOOST_OS_LINUX)
    int last_slash = path.find_last_of("/");
    path = path.substr(0, last_slash);
    std::string app_path = path + "/../testdata/app.xml";
    std::string deploy_path = path + "/../testdata/deploy.xml";
#endif
----------------------------------------------------------------*/

    std::string
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
