#ifndef REDUCE_H
#define REDUCE_H

#include "component.h"
#include <algorithm>
#include <numeric>

namespace alpha {
namespace protort {
namespace components {

/*!
 *\brief Компонент reduce - принимает сообшения со всех портов
 */
template <data (*T)(std::vector<data>&)>
class reduce : public component
{
public:
    reduce():
        is_relevant_(max_in_port_,false),values_(max_in_port_){}

    void process(port_id input_port, std::string const & payload) final override
    {
        output_list res;// Результат, который мы отправим
        output out;
        data d;

        values_[input_port] = d.unpack(payload);// Запись данных,пришедших из порта в вектор
        is_relevant_[input_port] = true;// Пометка о том, что мы прошли этот порт

        if(is_all_relevant())// Если все порты помечены(пройдены)
        {
            make_all_unrelevant();// Снимаем метки(устанавливаем в ячейки false)
            out.payload = T(values_).pack();// Пакуем все пришедшие данные в строку
            out.ports.push_back(0);
            res.push_back(out);//Сохраняем наши собрынне out-данные в результате


            router_ptr router = router_.lock();
            if (router)
                router->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                    router,
                                                    comp_inst_,
                                                    res));// Отправка результата
        }
    }

    port_id in_port_count() const final override { return max_in_port_; }
    port_id out_port_count() const final override { return 1; }

protected:

    /*!
     * \brief Проверяет состояние каждого элемента вектора
     * \return Если все ячейки true, то return true
     *         Иначе return false
     */
    bool is_all_relevant()
    {
        return std::all_of(is_relevant_.begin(),is_relevant_.end(),
                           [](bool p) -> bool
                           { return p;});
    }

    /*!
     * \brief Задает каждому элементу вектора значение false
     */
    void make_all_unrelevant()
    {
        std::fill(is_relevant_.begin(),is_relevant_.end(),false);
    }

    port_id max_in_port_ = 3;
    std::vector<bool> is_relevant_;// Вектор состояния портов. true - посещен, false - не посещен
    std::vector<data> values_;// Массив пришедших данных

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
