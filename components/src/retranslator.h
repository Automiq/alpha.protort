#ifndef RETRANSLATOR_H
#define RETRANSLATOR_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Компонент retranslator - ретранслирует данные.
*/

class retranslator : public component
{
public:
    //! \brief Привязываем роутер к компоненту
    retranslator(router_ptr router): component(router)
    {

    }
    /*!
     * \brief Ретранслирует данные(payload) в виде строки
     */
    void process(port_id input_port, std::string const & payload) final override
    {
        if (input_port == 0 || input_port == 1)
        {
            assert(comp_inst_ != nullptr);
            output_list data{ { payload, {0, 1} } };// Формируем выходной список с пришедшими данными

            router_ptr router = router_.lock();
            if (router)
                router->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                    router,
                                                    comp_inst_,
                                                    data));// Отправка данных
        }
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 2; }
    void start() final override { }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // RETRANSLATOR_H
