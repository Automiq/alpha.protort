#ifndef GENERATOR_H
#define GENERATOR_H

#include "component.h"
#include <boost/thread.hpp>
#include "router.h"

namespace alpha {
namespace protort {
namespace components {

class generator : public component
{
public:
    generator(node::router<node::node>& router, std::string name):
        component(router, name)
    {

    }

    output_list process(port_id input_port, std::string const & payload) final override
    {
        std::string rand_str("Random_string");
        return { { rand_str, { 0, 1 } } };
    }
    port_id in_port_count() const final override { return 0; }
    port_id out_port_count() const final override { return 2; }

    void generate()
    {
        // TODO generate meaningful data
        std::string data("Generated string");

        router_.do_route(name_, { { data, { 0, 1 } } });
    }

    void start() final override
    {
        // TODO fix new thread creation
        boost::thread new_thread{boost::bind(&generator::generate, this)};
    }
private:
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
