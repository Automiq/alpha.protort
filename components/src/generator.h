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
 * \brief Компонент generator - генерирует поток случайных чисел
 */
class generator : public component
{
public:
    //! \brief Привязываем роутер к компоненту, инициализируем генератор,
    //!
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

    /*!
     * \brief Генерирут случайные чисела  в объекте data, ковертирует объект в строку и передает ее в
     * router->get_service().post(boost::bind(&node::router<node::node>::do_route,
     *                                               router, comp_inst_, data));
     */
    void generate()
    {
        if (!started_)
            return;

        data d;
       /*!
        * Преобразование случайного беззнакового int,
        * сгенерированного gen, в double на интервале [0,100)
        * Каждый вызов dis_(gen_) генерирует новый рандомный float
        */
        d.val = dis_(gen_);
        time_mutex_.lock();
        d.time = std::time(NULL);// Фиксация времени
        time_mutex_.unlock();
        d.address = reinterpret_cast<uint64_t>(this);

        output_list data{ {d.pack() , {0 , 1}} };// Пакуем наши данные в строку и формируем выходной список

        router_ptr router = router_.lock();
        if (router)// Если удалось зафиксировать
            router->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                router,
                                                comp_inst_,
                                                data));// Отправка данных
        generate_next();// Вызов следующей итерации генератора
    }

    /*!
     * \brief Метод для организации перехода к следующей итерации генератора
     * Переопределяется в классе генератора с таймером timed_generator
     */
    virtual void generate_next()
    {
        router_ptr router = router_.lock();
        if (router)// Если удалось зафиксировать
            router->get_service().post(boost::bind(&generator::generate,
                                               boost::static_pointer_cast<generator>(this->shared_from_this())));
    }

    /*!
     * \brief Запускает процесс генерации
     */
    void start() final override
    {
        started_ = true;// Устанавливаем состояние начала генерации
        generate();
    }

    /*!
     * \brief Завершает процесс генерации
     */
    void stop()
    {
        started_ = false;// Устанавливаем состояние окончания генерации
    }

protected:
    boost::atomic_bool started_{false};// Пеменная состояния генерации
    //false - генератор запушен, true - генератор отключен
private:
    std::random_device rd_;// Будет использоваться для получения seed для механизма случайных чисел
    std::mt19937 gen_;// Standard mersenne_twister_engine seeded with rd();
    std::uniform_real_distribution<> dis_;// Производит случайные значения с плавающей запятой i, равномерно распределенные на интервале [a, b)
    boost::mutex time_mutex_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // GENERATOR_H
