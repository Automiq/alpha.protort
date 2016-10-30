#ifndef RETRANSLATOR_H
#define RETRANSLATOR_H

#include "i_component.h"

namespace alpha {
namespace protort {
namespace components {

class retranslator : public i_component
{
public:
    output_list process(port_id input_port, std::string const & payload) final override
    {
        if (input_port == 0 || input_port == 1)
        {
            output_list result = {{payload, {0, 1}}};
            std::cout << "retranslator string: " << payload << std::endl;
            return result;
        }
        std::vector<output> result;
        return result;
    }
    port_id input_port_count() const final override { return 2; }
    port_id output_port_count() const final override { return 2; }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // RETRANSLATOR_H
