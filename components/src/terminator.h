#ifndef TERMINATOR_H
#define TERMINATOR_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

class terminator : public component
{
public:
    terminator(router_ptr router): component(router)
    {

    }

    void process(port_id input_port, std::string const & payload) final override
    {

    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // TERMINATOR_H
