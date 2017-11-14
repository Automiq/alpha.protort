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

/*!
 * \brief Компонент factory - в зависимости от типа компонента, создает новый объект этого типа,
 * инициализируя его router, и возвращает shared_ptr, который владеет этим объектом.
 */

class factory
{
public:
    factory() = delete;

    /*!
     * \brief Cоздает shared_ptr, в управление которому отдается только что созданный экземпляр компонента
     * с типом kind, инициализируя его router
     */
    static component_ptr create(
            protocol::ComponentKind kind,
            router_ptr router)
    {
        component_ptr ptr;

        switch (kind) {
        case protocol::ComponentKind::Generator:
            // Отдаем во владение shared_ptr новый экземпляр типа компонента
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
        // Возвращение shared_ptr, владеющего экземпляром с типом компонента
        return ptr;
    }

    /*!
     * \brief Возвращает вызов create с параметрами:
     * Функция get_component_kind(kind), которая возвращает тип компомнента,
     * в зависимости от входной строки, которая описывает тип компонента
     * router , который реализует отправку, доставку и маршрутизацию данных между логическими портами
     */
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
