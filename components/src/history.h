#ifndef HISTORY_H
#define HISTORY_H

#include <fstream>

#include "data.h"
#include "component.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Ведет запись сообщений принятых на свои порты в файл history.txt в виде:
 * адресс << ' ' << значение << ' ' << время;
*/

class history : public component
{
public:
    //! \brief Привязываем роутер к компоненту и поток к файлу.
    history(router_ptr router) :
        component(router),
        f_("history.txt", std::ios_base::out)
    {
        f_ << std::fixed << std::setprecision(4);
    }

    /*!
     * \brief Делает распаковку строки в объект data и записывает в файл
     * состояние data т.е. address, value, time
     */
    void process(port_id input_port, std::string const & payload) final override
    {
        data d;
        d.unpack(payload);// d теперь содержит информацию, которую нес в себе payload

        boost::mutex::scoped_lock lock(fstream_mutex_);// Защита данных
        f_ << d.address << ' ' << d.val << ' ' << std::asctime(std::localtime(&d.time)) << std::flush;// Вывод данных из d в файл
        // Без std :: flush вывод будет таким же, но может не отображаться в реальном времени
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }

    /*!
     * \brief Останавливает запись в файл
     */
    void stop()
    {
        boost::mutex::scoped_lock lock(fstream_mutex_);// Защита данных
        f_ << std::flush;// Без std :: flush вывод будет таким же, но может не отображаться в реальном времени
    }

private:
    boost::mutex fstream_mutex_;
    std::fstream f_;// Поток в файл
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // HISTORY_H
