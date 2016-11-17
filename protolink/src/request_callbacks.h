#ifndef REQUEST_CALLBACKS_H
#define REQUEST_CALLBACKS_H

#include <boost/signals2.hpp>

#include "protocol.pb.h"

namespace alpha {
namespace protort {
namespace protolink {

class request_callbacks
{
public:
    boost::signals2::signal<void(protocol::Packet_Payload)> on_finished;
    boost::signals2::signal<void()> on_timeout;
    boost::signals2::signal<void()> on_abort;
};

} // namespace protolink
} // namespace protort
} // namespace alpha

#endif // REQUEST_CALLBACKS_H
