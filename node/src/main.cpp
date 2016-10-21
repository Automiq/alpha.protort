#include "node_settings.h"
#include "node.h"

using namespace alpha::protort::node;
int main(int argc, const char *argv[])
{
    node_settings node_settings_;
    if (!node_settings_.parse(argc, argv))
        return 1;

    auto ep = node_settings_.component_kind == alpha::protort::protocol::Packet::Generator ?
              node_settings_.destination : node_settings_.source;

    node n(ep, node_settings_.component_kind);
    n.start();

    return 0;
}
