#ifndef HISTORY_H
#define HISTORY_H

#include "component.h"
#include <fstream>
#include <iostream>

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
    history(node::router<node::node>& router) :
        component(router),
        f_("history.txt", std::ios_base::out)
    {

    }

    void process(port_id input_port, std::string const & payload) final override
    {
        data d;
        d.unpack(payload);

        f_ << d.val << ' ' << std::asctime(std::localtime(&d.time));
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 0; }
private:
    std::fstream f_;
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // HISTORY_H
