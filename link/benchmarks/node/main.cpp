#include "node_settings.h"
#include "node.h"

using namespace alpha::protort::link::benchmarks;

int main(int argc, const char *argv[])
{
    node_settings node_settings_;
    if (!node_settings_.parse(argc, argv))
        return 1;

    node n(node_settings_);
    n.start();

    return 0;
}
