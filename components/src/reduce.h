#ifndef REDUCE_H
#define REDUCE_H

#include "component.h"

#include <string>

namespace alpha {
namespace protort {
namespace components {

template <float (*T)(std::vector<float>&)>
class reduce : public component
{
public:
    reduce():
        is_relevant(max_in_port,false),values(max_in_port){}

    output_list process(port_id input_port, std::string const & payload) final override
    {
        output_list res;
        output out;
        float res_val=0;

        values[input_port] = std::stof(payload);
        is_relevant[input_port] = true;

        if(is_all_relevant())
        {
            make_all_unrelevant();
            res_val = T(values);
            out.payload = std::to_string(res_val);
            out.ports.push_back(0);
            res.push_back(out);
        }

        return res;
    }

    port_id in_port_count() const final override { return max_in_port; }
    port_id out_port_count() const final override { return 1; }

protected:

    bool is_all_relevant()
    {
        for(bool rel: is_relevant)
            if (!rel) return false;
        return true;
    }

    void make_all_unrelevant()
    {
        for(bool &&rel: is_relevant)
            rel=false;
    }

    port_id max_in_port = 3;
    std::vector<bool> is_relevant;
    std::vector<float> values;

};

float do_max(std::vector<float> &v)
{
    float max;
    max = v[0];
    for(float val: v)
        max = val > max ? val : max;
    return max;
}

using max = reduce<do_max>;

float do_min(std::vector<float> &v)
{
    float min;
    min = v[0];
    for(float val: v)
        min = val < min ? val : min;
    return min;
}

using min = reduce<do_min>;

float do_average(std::vector<float> &v)
{
    float sum;
    for(float val: v)
        sum += val;
    return sum / v.size();
}

using average = reduce<do_average>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // REDUCE_H
