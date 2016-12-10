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
 * \brief Генератор, генерирует поток случайных чисел
 */
class timed_generator : public generator
{
public:
    timed_generator(router_ptr& router):
        generator(router),
        generate_interval_(100),
        generate_timer_(router->get_service())
    {

    }

    void generate_next()
    {
        generate_timer_.expires_from_now(boost::posix_time::milliseconds(generate_interval_));
        generate_timer_.async_wait(boost::bind(&timed_generator::generate,
                                                   boost::static_pointer_cast<timed_generator>(this->shared_from_this())));
    }

    void stop()
    {
        started_ = false;
        generate_timer_.cancel();
    }

private:
    int generate_interval_;
    boost::asio::deadline_timer generate_timer_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
