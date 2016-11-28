#ifndef HISTORY_H
#define HISTORY_H

#include "component.h"
#include <fstream>

namespace alpha {
namespace protort {
namespace components {

class history : public component
{
public:
    history(node::router<node::node>& router): component(router),f("history.txt")
    {

    }

    void process(port_id input_port, std::string const & payload) final override
    {
        f << payload << std::endl;
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }
    void start() final override { }
    void stop() final override { }
private:
    std::fstream f;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // HISTORY_H
