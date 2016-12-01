#ifndef RETRANSLATOR_H
#define RETRANSLATOR_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

class retranslator : public component
{
public:
    retranslator(node::router<node::node>& router): component(router)
    {

    }
    void process(port_id input_port, std::string const & payload) final override
    {
        if (input_port == 0 || input_port == 1)
        {
            assert(comp_inst_ != nullptr);
            router_.do_route(comp_inst_, { { payload, {0, 1} } });
        }
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 2; }
    void start() final override { }
    void stop() final override { }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // RETRANSLATOR_H
