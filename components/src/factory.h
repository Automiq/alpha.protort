#ifndef ALPHA_PROTORT_COMPONENTS_FACTORY_H
#define ALPHA_PROTORT_COMPONENTS_FACTORY_H

#include "convert.h"
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
namespace node {
class node;
template<class app> class router;
}
}
}

namespace alpha {
namespace protort {
namespace components {

class factory
{
public:
    factory() = delete;

    static component_ptr create(
            protocol::ComponentKind kind,
            router_ptr router)
    {
        component_ptr ptr;

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
        case protocol::ComponentKind::Calc:
            ptr.reset(new components::calc(router));
            break;
        case protocol::ComponentKind::History:
            ptr.reset(new components::history(router));
            break;
        case protocol::ComponentKind::TimedGenerator:
            ptr.reset(new components::timed_generator(router));
            break;
        default:
            assert(false);
            break;
        }

        return ptr;
    }

    static component_ptr create(
            const std::string& kind,
            router_ptr router)
    {
        return create(get_component_kind(kind), router);
    }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENTS_FACTORY_H
