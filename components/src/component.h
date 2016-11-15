#ifndef ALPHA_PROTORT_COMPONENT_H
#define ALPHA_PROTORT_COMPONENT_H

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

class component
{
public:
    virtual output_list process(port_id input_port, std::string const& payload) = 0;
    virtual port_id in_port_count() const = 0;
    virtual port_id out_port_count() const = 0;

    output_list do_process(port_id input_port, std::string const& payload)
    {
        ++in_packet_count_;
        output_list result = process(input_port, payload);
        out_packet_count_ += result.size();
        return result;
    }

    uint32_t in_packet_count() const
    {
        return in_packet_count_;
    }

    uint32_t out_packet_count() const
    {
        return out_packet_count_;
    }

protected:
    uint32_t in_packet_count_ = 0;
    uint32_t out_packet_count_ = 0;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENT_H
