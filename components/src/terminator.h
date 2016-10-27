#ifndef TERMINATOR_H
#define TERMINATOR_H

#include "i_component.h"

namespace alpha {
namespace protort {
namespace components {


class terminator : public i_component
{
public:
    output_list process(port_id input_port, std::string const & payload) final override
    {
        return {};
    }
    port_id in_ports() const final override { return 2; }
    port_id out_ports() const final override { return 0; }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // TERMINATOR_H
