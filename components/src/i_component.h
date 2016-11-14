#ifndef I_COMPONENT_H
#define I_COMPONENT_H

#include <iostream>
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
    virtual port_id in_port_count() const = 0;
    virtual port_id out_port_count() const = 0;

    output_list do_process(port_id input_port, std::string const & payload)
    {
        counter_processed_packets++;
        return process(input_port, payload);
    }

    uint32_t get_counter_processed_packets() const
    {
        return counter_processed_packets;
    }
protected:
    uint32_t counter_processed_packets = 0;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // I_COMPONENT_H
