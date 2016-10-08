#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "packet.pb.h"

namespace alpha {
namespace protort {
namespace protocol {
namespace tests {

namespace _app = alpha::protort::protocol;

BOOST_AUTO_TEST_SUITE(tests_packet)

BOOST_AUTO_TEST_CASE(test_serialize_parse)
{
    const uint32_t value = 42;

    _app::Packet packet;
    packet.set_foobar(value);

    BOOST_CHECK_EQUAL(packet.foobar(), value);

    auto serializedAsString = packet.SerializeAsString();

    _app::Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedAsString);

    BOOST_CHECK_EQUAL(deserializedPacket.foobar(), value);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
