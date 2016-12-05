#ifndef GENERATOR_H
#define GENERATOR_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <random>

#include "component.h"
#include "router.h"
#include "data.h"


namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Генератор, генерирует поток случайных чисел
 */
class generator : public component
{
public:
    generator(router_ptr router):
        component(router),
        gen_(rd_()),
        dis_(0,100)
    {

    }

    void process(port_id input_port, std::string const & payload) final override
    {

    }
    port_id in_port_count() const final override { return 0; }
    port_id out_port_count() const final override { return 2; }

    void generate()
    {
        if (!started_)
            return;

        data d;
        d.val = dis_(gen_);
        d.time = std::time(NULL);

        router_.do_route(comp_inst_,{ {d.pack() , {0 , 1}} });
        generate_next();
    }

    /*!
     * \brief Метод для организации перехода к следующей итерации генератора
     * Переопределяется в классе генератора с таймером
     */
    virtual void generate_next()
    {
        router_.get_service().post(boost::bind(&generator::generate,
                                               boost::static_pointer_cast<generator>(this->shared_from_this())));
    }

    void start() final override
    {
        started_ = true;
        generate();
    }

    void stop()
    {
        started_ = false;
    }

protected:
    bool started_;

private:
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<> dis_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
