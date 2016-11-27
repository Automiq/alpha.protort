#ifndef ALPHA_PROTORT_COMPONENTS_FACTORY_H
#define ALPHA_PROTORT_COMPONENTS_FACTORY_H

#include "components.h"
#include "packet.pb.h"

// node, router forward declaration
namespace alpha {
namespace protort {
namespace node {
class node;
template<class app> class router;
} // node
} // protort
} // alpha

namespace alpha {
namespace protort {
namespace components {

class factory
{
public:
    factory() = delete;

    static protocol::ComponentKind get_component_kind(const std::string& kind)
    {
        if (kind == "generator")
            return protocol::ComponentKind::Generator;
        else if (kind == "retranslator")
            return protocol::ComponentKind::Retranslator;
        else if (kind == "terminator")
            return protocol::ComponentKind::Terminator;
        else
            assert(false);
    }

    static std::string get_component_kind(const protocol::ComponentKind& kind)
    {
        switch (kind) {
        case protocol::ComponentKind::Generator:
            return "generator";
        case protocol::ComponentKind::Retranslator:
            return "retranslator";
        case protocol::ComponentKind::Terminator:
            return "terminator";
        default:
            assert(false);
            return {};
        }
    }

    static std::shared_ptr<components::component> create(protocol::ComponentKind kind,
                                                         node::router<node::node>& router)
    {
        std::shared_ptr<components::component> ptr;

        switch (kind) {
        case protocol::ComponentKind::Generator:
            ptr.reset(new components::generator(router));
            break;
        case protocol::ComponentKind::Retranslator:
            ptr.reset(new components::retranslator(router));
            break;
        case protocol::ComponentKind::Terminator:
            ptr.reset(new components::terminator(router));
            break;
        default:
            assert(false);
            break;
        }

        return ptr;
    }

    static std::shared_ptr<components::component> create(const std::string& kind,
                                                         node::router<node::node>& router)
    {
        return create(get_component_kind(kind), router);
    }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENTS_FACTORY_H
