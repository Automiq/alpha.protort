#ifndef RETRANSLATOR_H
#define RETRANSLATOR_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

class retranslator : public component
{
public:
    retranslator(router_ptr router): component(router)
    {

    }
    void process(port_id input_port, std::string const & payload) final override
    {
        if (input_port == 0 || input_port == 1)
        {
            assert(comp_inst_ != nullptr);
            output_list data{ { payload, {0, 1} } };

            if (!router_.expired())
                router_.lock()->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                    router_.lock(),
                                                    comp_inst_,
                                                    data));
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
