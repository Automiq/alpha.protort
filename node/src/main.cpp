#include "node_settings.h"
#include "node.h"
#include "node_deploy.h"

using namespace alpha::protort;
int main(int argc, const char *argv[])
{
    node::node_settings node_settings_;
    if (!node_settings_.parse(argc, argv))
        return 1;


    node::node n(node_settings_);
    n.start();

    return 0;
}
