#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <iostream>

#include "packet.pb.h"

using namespace std;
using namespace alpha::protort::protocol;

namespace alpha {
namespace protort {
namespace protocol {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_packet)

BOOST_AUTO_TEST_CASE(test_serialize_parse)
{
    const uint32_t id = 1;
    Packet newpacket;
    newpacket.set_id(id);
    BOOST_CHECK_EQUAL(newpacket.id(), id);
    newpacket.set_component(Packet::Generator);
    BOOST_CHECK_EQUAL(newpacket.component(), Packet::Generator);

    auto serializedPayloadAsStr = newpacket.SerializeAsString();

    newpacket.set_allocated_payload(&serializedPayloadAsStr);

    BOOST_CHECK_EQUAL(newpacket.payload(), serializedPayloadAsStr);

    auto serializedPacketAsStr = newpacket.SerializeAsString();

    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacketAsStr);
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), serializedPayloadAsStr);
    BOOST_CHECK_EQUAL(deserializedPacket.component(), Packet::Generator);
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);
    cin.get();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
