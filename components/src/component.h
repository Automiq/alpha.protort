#ifndef ALPHA_PROTORT_COMPONENT_H
#define ALPHA_PROTORT_COMPONENT_H

#include <iostream>
#include <vector>
#include <memory>
#include <ctime>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

// node, router forward declaration
namespace alpha {
namespace protort {
namespace node {
class node;
template<class app> class router;
} // node
} // protort
} // alpha

namespace alpha {
namespace protort {
namespace components {

using port_id = uint32_t;
using port_list = std::vector<port_id>;

class component;

using component_ptr = boost::shared_ptr<component>;
using router_ptr = boost::shared_ptr<node::router<node::node>>;
using router_weak_ptr = boost::weak_ptr<node::router<node::node>>;

struct output
{
    std::string payload;
    port_list ports;
};

using output_list = std::vector<output>;

class component : public boost::enable_shared_from_this<component>
{
public:
    component(router_ptr router):
        router_(router)
    {

    }
    virtual void process(port_id input_port, std::string const& payload) = 0;
    virtual port_id in_port_count() const = 0;
    virtual port_id out_port_count() const = 0;
    virtual void start() { }
    virtual void stop() { }

    void do_process(port_id input_port, std::string const& payload)
    {
        ++in_packet_count_;
        process(input_port, payload);
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
    boost::atomic_uint32_t in_packet_count_{0};
    router_weak_ptr router_;
    void *comp_inst_ = nullptr;

};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // ALPHA_PROTORT_COMPONENT_H
