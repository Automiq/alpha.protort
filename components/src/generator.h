#ifndef GENERATOR_H
#define GENERATOR_H

#include "i_component.h"

namespace alpha {
namespace protort {
namespace components {

class generator : public i_component
{
public:
    output_list process(port_id input_port, std::string const & payload) final override
    {
        std::string rand_str("Random_string");
        return { { rand_str, { 0, 1 } } };
    }
    port_id in_port_count() const final override { return 0; }
    port_id out_port_count() const final override { return 2; }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
