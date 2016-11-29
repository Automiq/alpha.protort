#ifndef GENERATOR_H
#define GENERATOR_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "component.h"
#include "router.h"

namespace alpha {
namespace protort {
namespace components {

class generator : public component
{
public:
    generator(node::router<node::node>& router):
        component(router),
        generate_interval_(3000),
        generate_timer_(router.get_service())
    {

    }

    void process(port_id input_port, std::string const & payload) final override
    {

    }
    port_id in_port_count() const final override { return 0; }
    port_id out_port_count() const final override { return 2; }

    void generate()
    {
        if (!started_)
            return;

        // TODO generate meaningful data
        data d;
        d.val = rand(100,0,3);
        d.time = std::time(NULL);
        router_.do_route(comp_inst_,{ {d.pack() , {0 , 1}} });

        generate_timer_.expires_from_now(boost::posix_time::milliseconds(generate_interval_));
        generate_timer_.async_wait(boost::bind(&generator::generate, std::static_pointer_cast<generator>(shared_from_this())));
    }

    void start() final override
    {
        started_ = true;
        generate();
    }

    void stop() final override
    {
        started_ = false;
        generate_timer_.cancel();
    }
private:

    float rand(int max,int min,int chars_after_point)
    {
        int div = std::pow(10,chars_after_point);
        return float((std::rand() + min * div) % (max * div)) / div;
    }

    boost::asio::deadline_timer generate_timer_;
    int generate_interval_;
    bool started_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
