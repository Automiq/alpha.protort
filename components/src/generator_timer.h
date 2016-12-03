#ifndef GENERATOR_TIMER_H
#define GENERATOR_TIMER_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "component.h"
#include "router.h"
#include "generator.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Генератор, генерирует поток случайных чисел
 */
class generator_timer : public generator
{
public:
    generator_timer(node::router<node::node>& router):
        generator(router),
        generate_interval_(3000),
        generate_timer_(router.get_service())
    {

    }

    void generate_next()
    {
        std::cout << "going to next generation with timer" << std::endl;
        generate_timer_.expires_from_now(boost::posix_time::milliseconds(generate_interval_));
        generate_timer_.async_wait(boost::bind(&generator_timer::generate,
                                                   boost::static_pointer_cast<generator_timer>(this->shared_from_this())));
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
