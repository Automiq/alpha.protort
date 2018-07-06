#include "node_settings.h"
#include "node.h"
#include <clocale>

using namespace alpha::protort;
int main(int argc, const char *argv[])
{
    setlocale(LC_ALL, "rus");
    node::node_settings settings;
    if (!settings.parse(argc,argv))
        return 1;

    auto n = boost::make_shared<node::node>(settings);
    n->start();

    return 0;
}
