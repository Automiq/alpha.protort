#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "packet.pb.h"

using namespace alpha::protort::protocol;

namespace alpha {
namespace protort {
namespace protocol {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_packet)

BOOST_AUTO_TEST_CASE(test_serialize_parse)
{
    const uint32_t id = 1;
    const Packet::component_kind component = Packet::Generator;
    const std::string str = "afwrger";
    Packet newpacket;
    newpacket.set_id(id);
    BOOST_CHECK_EQUAL(newpacket.id(), id);
    newpacket.set_component(component);
    BOOST_CHECK_EQUAL(newpacket.component(), component);

    newpacket.set_payload(str);

    BOOST_CHECK_EQUAL(newpacket.payload(), str);

    auto serializedPacketAsStr = newpacket.SerializeAsString();

    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacketAsStr);
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), str);
    BOOST_CHECK_EQUAL(deserializedPacket.component(), component);
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
