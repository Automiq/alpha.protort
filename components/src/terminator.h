#ifndef TERMINATOR_H
#define TERMINATOR_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

class terminator : public component
{
public:
    terminator(node::router<node::node>& router): component(router)
    {

    }

    output_list process(port_id input_port, std::string const & payload) final override
    {
        return { };
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }
    void start() final override { }
    void stop() final override { }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // TERMINATOR_H
