#ifndef ALPHA_PROTORT_COMPONENTS_CONVERT_H
#define ALPHA_PROTORT_COMPONENTS_CONVERT_H

namespace alpha {
namespace protort {
namespace components {

    static protocol::ComponentKind get_component_kind(const std::string& kind)
    {
        if (kind == "generator")
            return protocol::ComponentKind::Generator;
        else if (kind == "retranslator")
            return protocol::ComponentKind::Retranslator;
        else if (kind == "terminator")
            return protocol::ComponentKind::Terminator;
        else if (kind == "calc")
            return protocol::ComponentKind::Calc;
        else if (kind == "history")
            return protocol::ComponentKind::History;
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
        case protocol::ComponentKind::Calc:
            return "calc";
        case protocol::ComponentKind::History:
            return "history";
        default:
            assert(false);
            return {};
        }
    }

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENTS_CONVERT_H
