#ifndef CALC_H
#define CALC_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdlib>
#include <sstream>

#include "router.h"
#include "component.h"
#include "data.h"

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Calc- принимает сообщение, обрабатывает и передаёт дальше
 * (на данный момент просто умножает значение на 100)
 */
class calc: public component
{
public:
    calc(router_ptr router): component(router)
    {

    }
    void process(port_id input_port, std::string const & payload) final override
    {
        output_list out(1);
        data d;

        d.unpack(payload);
        d.val *= 100;
        out[0].payload = d.pack();
        out[0].ports.push_back(0);

        router_ptr router = router_.lock();
        if (router)
            router->get_service().post(boost::bind(&node::router<node::node>::do_route,
                                                    router,
                                                    comp_inst_,
                                                    out));
    }
    port_id in_port_count() const final override { return 2; }
    port_id out_port_count() const final override { return 2; }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // CALC_H
