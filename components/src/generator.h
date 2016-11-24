#ifndef GENERATOR_H
#define GENERATOR_H

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

class generator : public component
{
public:
    generator(node::router<node::node>& router):
        component(router),
        generating_interval(3000),
        timer(router.get_service())
    {
        std::srand(std::time(NULL));
    }

    output_list process(port_id input_port, std::string const & payload) final override
    {
        //std::string rand_str("Random_string");

        std::ostringstream o;
        o << rand(100,0,3) << ' ' << std::time(NULL);

        return { { o.str(), { 0, 1 } } };
    }
    port_id in_port_count() const final override { return 0; }
    port_id out_port_count() const final override { return 2; }

    void generate()
    {
        if (!started)
            return;

        // TODO generate meaningful data
        std::ostringstream o;
        o << rand(100,0,3) << ' ' << std::time(NULL);
        //std::string res(std::to_string(rand(100,0,3)) + std::to_string(std::time(NULL)));


        router_.do_route(comp_inst_,{ {o.str() , {0 , 1}} });

        timer.expires_from_now(boost::posix_time::milliseconds(generating_interval));
        timer.async_wait(boost::bind(&generator::generate, this));
    }

    void start() final override
    {
        started = true;
        generate();
    }

    void stop() final override
    {
        started = false;
    }
private:

    float rand(int max,int min,int chars_after_point)
    {
        int div = std::pow(10,chars_after_point);
        return float((std::rand() + min * div) % (max * div)) / div;
    }

    boost::asio::deadline_timer timer;
    int generating_interval;
    bool started;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
