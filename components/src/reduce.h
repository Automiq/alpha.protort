#ifndef REDUCE_H
#define REDUCE_H

#include "component.h"
#include <algorithm>
#include <numeric>

namespace alpha {
namespace protort {
namespace components {

/*!
 *\brief Шаблонный клас reduce
 * Принимает сообшения со всех портов
 */
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
            router_.lock()->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                    router_.lock(),
                                                    comp_inst_,
                                                    res));
        }
    }

    port_id in_port_count() const final override { return max_in_port_; }
    port_id out_port_count() const final override { return 1; }

protected:

    bool is_all_relevant()
    {
        return std::all_of(is_relevant_.begin(),is_relevant_.end(),
                           [](bool p) -> bool
                           { return p;});
    }

    void make_all_unrelevant()
    {
        std::fill(is_relevant_.begin(),is_relevant_.end(),false);
    }

    port_id max_in_port_ = 3;
    std::vector<bool> is_relevant_;
    std::vector<data> values_;

};

static data do_max(std::vector<data> &v)
{
    return *std::max_element(v.begin(),v.end(),
                            [](data a, data b) -> bool
                            {return a.val < b.val;});
}

//!\brief Вычисляет максимальное значение для пакетов всех портов
//! метка времени у отправляемого пакета совпадает с временем в
//! во входящем пакете с максимальным значением
using max = reduce<do_max>;

static data do_min(std::vector<data> &v)
{
    return *std::min_element(v.begin(),v.end(),
                            [](data a, data b) -> bool
                            {return a.val < b.val;});
}

//!\brief Вычисляет минимальное значение для пакетов всех портов
//! метка времени у отправляемого пакета совпадает с временем в
//! во входящем пакете с минимальным значением
using min = reduce<do_min>;

static data do_average(std::vector<data> &v)
{
    return std::accumulate(v.begin(),v.end(),
                           data({0,std::time(NULL)}),
                           [](data a, data b) -> data
                               {return {a.val + b.val, a.time};});
}

//!\brief Вычисляет среднее значение для пакетов всех портов
//! метка времени у отправляемого пакета равна моменту вычисления
//! среднего значения
using average = reduce<do_average>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // REDUCE_H
