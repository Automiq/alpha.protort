#include "node_settings.h"
#include "node_class.h"

using namespace alpha::protort::node;
int main(int argc, const char *argv[])
{
    node_settings node_settings;
    if (!node_settings.parse(argc, argv))
        return 1;

    auto ep = node_settings.component_kind == Packet::ComponentKind::Packet_ComponentKind_Generator ?
                node_settings.destination : node_settings.source;

    node n(ep, node_settings.component_kind);
    n.start();

    // TODO

    return 0;
}
