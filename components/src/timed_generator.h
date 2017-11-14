#ifndef GENERATOR_TIMER_H
#define GENERATOR_TIMER_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "component.h"
#include "router.h"
#include "generator.h"
#include "data.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Компонент generator - генерирует поток случайных чисел
 */
class timed_generator : public generator
{
public:
    //! \brief // Привязываем роутер к компоненту, задаем промежуток времени генерации, инициализируем таймер
    timed_generator(router_ptr& router):
        generator(router),
        generate_interval_(100),
        generate_timer_(router->get_service())
    {

    }
    /*!
     * \brief Каждый промежуток времени(generate_interval_) генерирует число
     */
    void generate_next()
    {
        generate_timer_.expires_from_now(boost::posix_time::milliseconds(generate_interval_));
        generate_timer_.async_wait(boost::bind(&timed_generator::generate,
                                                   boost::static_pointer_cast<timed_generator>(this->shared_from_this())));
    }

    void stop()
    {
        started_ = false;// Сигнализируем об окончании генерации
        generate_timer_.cancel();
    }

private:
    int generate_interval_;// Промежуток времени для генерации
    boost::asio::deadline_timer generate_timer_;// Таймер
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
