#ifndef I_COMPONENT_H
#define I_COMPONENT_H

#include <iostream>
#include <string>
#include <vector>

namespace alpha {
namespace protort {
namespace components {

using port_id = unsigned short;
using port_list = std::vector<port_id>;

struct output
{
    std::string payload;
    port_list ports;
};

using output_list = std::vector<output>;

class i_component
{
public:
    virtual output_list process(port_id input_port, std::string const & payload) = 0;
    virtual port_id in_ports() const = 0;
    virtual port_id out_ports() const = 0;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // I_COMPONENT_H
