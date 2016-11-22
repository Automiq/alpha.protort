#ifndef ALPHA_PROTORT_COMPONENT_H
#define ALPHA_PROTORT_COMPONENT_H

#include <iostream>
#include <vector>

namespace alpha {
namespace protort {
namespace node {
class node;
template<class app> class router;
}
}
}

namespace alpha {
namespace protort {
namespace components {

using port_id = uint32_t;
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
    component(node::router<node::node>& router):
        router_(router)
    {

    }
    virtual output_list process(port_id input_port, std::string const& payload) = 0;
    virtual port_id in_port_count() const = 0;
    virtual port_id out_port_count() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    output_list do_process(port_id input_port, std::string const& payload)
    {
        ++in_packet_count_;
        output_list result = process(input_port, payload);
        return result;
    }

    uint32_t in_packet_count() const
    {
        return in_packet_count_;
    }
    void set_comp_inst(void *comp_inst)
    {
        comp_inst_ = comp_inst;
    }

protected:
    uint32_t in_packet_count_ = 0;
    node::router<node::node>& router_;
    void *comp_inst_;

};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENT_H
