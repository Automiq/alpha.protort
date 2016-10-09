#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <iostream>

#include "packet.pb.h"

namespace _app = alpha::protort::protocol;
using namespace std;
using namespace _app;

namespace alpha {
namespace protort {
namespace protocol {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_packet)

BOOST_AUTO_TEST_CASE(test_serialize_parse)
{
    const uint32_t id = 1;
    string str0 = "afegrgb";
    string str1 = "aevetbfegrgb";
    cout << "Welcome to test exe (Zhuravlev SS):" << endl;
    Packet newpacket;
    text newtext;
    newpacket.set_id(id);
    BOOST_CHECK_EQUAL(newpacket.id(), id);
    newpacket.set_component(Packet::CalcSin);
    BOOST_CHECK_EQUAL(newpacket.component(), Packet::CalcSin);
    newtext.add_mes(str0.c_str());
    newtext.add_mes(str1.c_str());
    BOOST_CHECK_EQUAL(newtext.mes(0), str0);
    BOOST_CHECK_EQUAL(newtext.mes(1), str1);

    auto serializedTextAsStr = newtext.SerializeAsString();

    text deserializedtext;
    deserializedtext.ParseFromString(serializedTextAsStr);

    BOOST_CHECK_EQUAL(deserializedtext.mes(0), str0);
    BOOST_CHECK_EQUAL(deserializedtext.mes(1), str1);

    newpacket.set_allocated_masbytes(&serializedTextAsStr);

    auto serializedPacketAsStr = newpacket.SerializeAsString();

    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacketAsStr);
    BOOST_CHECK_EQUAL(deserializedPacket.masbytes(), serializedTextAsStr);
    BOOST_CHECK_EQUAL(deserializedPacket.component(), Packet::CalcSin);
    cout << deserializedPacket.id()<< endl;
    cout << id << endl;
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);

    cout << "Everything is well done" << endl;
    cin.get();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
