#ifndef SENSOR_H
#define SENSOR_H

#include "component.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

namespace alpha {
namespace protort {
namespace components {

class sensor: public component
{
public:

    void process(port_id input_port, const std::string &payload) final override
    {
        output_list res(1);

        if (payload == "shutdown") is_down = true;
        if (is_down) return;

        std::srand(uint32_t(std::time(NULL)));

        res[0].payload = std::to_string(rand(max_val,min_val,after_point));
        res[0].ports.push_back(0);

        if (comp_inst_ != nullptr)
            router_.do_route(comp_inst_, res);
    }

    port_id in_port_count() const final override { return 1; }
    port_id out_port_count() const final override { return 1; }

private:
    float rand(int max,int min,int chars_after_point)
    {
        int div = std::pow(10,chars_after_point);
        return float((std::rand() + min * div) % (max * div)) / div;
    }

    int after_point = 4;
    int max_val = 60;
    int min_val = 0;
    bool is_down = false;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // SENSOR_H
