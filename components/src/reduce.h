#ifndef REDUCE_H
#define REDUCE_H

#include "component.h"

namespace alpha {
namespace protort {
namespace components {

template <data (*T)(std::vector<data>&)>
class reduce : public component
{
public:
    reduce():
        is_relevant_(max_in_port_,false),values_(max_in_port_){}

    void process(port_id input_port, std::string const & payload) final override
    {
        output_list res;
        output out;
        data d;

        values_[input_port] = d.unpack(payload);
        is_relevant_[input_port] = true;

        if(is_all_relevant())
        {
            make_all_unrelevant();
            out.payload = T(values_).pack();
            out.ports.push_back(0);
            res.push_back(out);
            router_.do_route(comp_inst_,res);
            return;
        }else
            return;

    }

    port_id in_port_count() const final override { return max_in_port_; }
    port_id out_port_count() const final override { return 1; }

protected:

    bool is_all_relevant()
    {
        for(bool rel: is_relevant_)
            if (!rel) return false;
        return true;
    }

    void make_all_unrelevant()
    {
        for(bool &&rel: is_relevant_)
            rel=false;
    }

    port_id max_in_port_ = 3;
    std::vector<bool> is_relevant_;
    std::vector<data> values_;

};

static data do_max(std::vector<data> &v)
{
    data max = v[0];
    for(data &d: v)
        if(d.val > max.val) max = d;
    return max;
}

using max = reduce<do_max>;

static data do_min(std::vector<data> &v)
{
    data min = v[0];
    for(data &d: v)
        if(d.val < min.val) min = d;
    return min;
}

using min = reduce<do_min>;

static data do_average(std::vector<data> &v)
{
    float sum;
    data res;
    for(data &d: v)
        sum += d.val;
    res.val = sum / v.size();
    res.time = std::time(NULL);
    return res;
}

using average = reduce<do_average>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // REDUCE_H
