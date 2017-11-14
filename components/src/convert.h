#ifndef ALPHA_PROTORT_COMPONENTS_CONVERT_H
#define ALPHA_PROTORT_COMPONENTS_CONVERT_H

namespace alpha {
namespace protort {
namespace components {
    /*!
    * \brief Компонент convert - конвертирует строку
    * в тип компонента и наоборот.
    */

    /*!
     * \brief Возвращает тип компонента, соответствующий описанию kind.
     * kind - строка, содержимое которой является описанием типа компонента
     */
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
        else if (kind == "timed_generator")
            return protocol::ComponentKind::TimedGenerator;
        else
            assert(false);
    }
    /*!
     * \brief Возвращает строку, соответствующую содержимому kind.
     * kind - тип компонента
     */
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
        case protocol::ComponentKind::TimedGenerator:
            return "timed_generator";
        default:
            assert(false);
            return {};
        }
    }

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENTS_CONVERT_H
