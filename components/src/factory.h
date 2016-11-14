#ifndef ALPHA_PROTORT_COMPONENTS_FACTORY_H
#define ALPHA_PROTORT_COMPONENTS_FACTORY_H

#include "components.h"
#include "packet.pb.h"

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
            return {"generator"};
        case protocol::ComponentKind::Retranslator:
            return {"retranslator"};
        case protocol::ComponentKind::Terminator:
            return {"terminator"};
        default:
            assert(false);
            return {};
        }
    }

    static std::unique_ptr<components::i_component> create(protocol::ComponentKind kind)
    {
        std::unique_ptr<components::i_component> ptr;

        switch (kind) {
        case protocol::ComponentKind::Generator:
            ptr.reset(new components::generator);
            break;
        case protocol::ComponentKind::Retranslator:
            ptr.reset(new components::retranslator);
            break;
        case protocol::ComponentKind::Terminator:
            ptr.reset(new components::terminator);
            break;
        default:
            assert(false);
            break;
        }

        return ptr;
    }

    static std::unique_ptr<components::i_component> create(const std::string& kind)
    {
        return create(get_component_kind(kind));
    }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENTS_FACTORY_H
