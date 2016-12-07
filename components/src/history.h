#ifndef HISTORY_H
#define HISTORY_H

#include <fstream>

#include "data.h"
#include "component.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Компонент "history"
 * ведёт запись сообщений, принятых на свои порты
 */
class history : public component
{
public:
    history(router_ptr router) :
        component(router),
        f_("history.txt", std::ios_base::out)
    {
        f_ << std::fixed << std::setprecision(4);
    }

    void process(port_id input_port, std::string const & payload) final override
    {
        data d;
        d.unpack(payload);

        boost::mutex::scoped_lock lock(fstream_mutex_);
        f_ << d.val << ' ' << std::asctime(std::localtime(&d.time));
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }
    void stop()
    {
        boost::mutex::scoped_lock lock(fstream_mutex_);
        f_ << std::flush;
    }

private:
    boost::mutex fstream_mutex_;
    std::fstream f_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // HISTORY_H
