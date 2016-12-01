#include "node_settings.h"
#include "node.h"

using namespace alpha::protort;
int main(int argc, const char *argv[])
{
    node::node_settings settings;
    if (!settings.parse(argc,argv))
        return 1;

    node::node n;
    n.start();

    return 0;
}
