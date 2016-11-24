#ifndef CALC_H
#define CALC_H

#include "component.h"
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstring>
#include <cstdlib>
#include <sstream>

#include "router.h"

namespace alpha {
namespace protort {
namespace components {

class calc: public component
{
public:
    calc(node::router<node::node>& router): component(router)
    {

    }
    output_list process(port_id input_port, std::string const & payload) final override
    {
        float val;
        unsigned int t;
        output_list out(1);
        std::ostringstream o;
        std::istringstream i(payload);

        i >> val >> t;
        val *= 100;

        o << val << ' '  << t << ' ' << input_port;
        out[0].payload = o.str();
        out[0].ports.push_back(0);

        if (comp_inst_ != nullptr)
            router_.do_route(comp_inst_, out);
        return out;
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 2; }
    void start() final override { }
    void stop() final override { }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // CALC_H
