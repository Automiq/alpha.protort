#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/scope_exit.hpp>

#include "packet.pb.h"

namespace alpha {
namespace protort {
namespace protocol {
namespace tests {

using namespace alpha::protort::protocol;

struct fixture
{
    ~fixture()
    {
        // Данный код будет вызван по окончании каждого теста, определенного
        // в рамках одного test suite.
        google::protobuf::ShutdownProtobufLibrary();
    }
};

BOOST_FIXTURE_TEST_SUITE(tests_packet, fixture)

BOOST_AUTO_TEST_CASE(test_serialize_parse)
{
    // Исходные данные
    const uint32_t id = 1;
    const Packet::ComponentKind componentKind = Packet::Generator;
    const std::string payload = "payload";

    // Пакет
    Packet packet;

    // Формируем пакет
    packet.set_id(id);
    packet.set_component_kind(componentKind);
    packet.set_payload(payload);

    // Проверяем корректность сформированного пакета
    BOOST_CHECK_EQUAL(packet.id(), id);
    BOOST_CHECK_EQUAL(packet.component_kind(), componentKind);
    BOOST_CHECK_EQUAL(packet.payload(), payload);

    // Сериализуем пакет в строку
    auto serializedPacket = packet.SerializeAsString();

    // Десериализуем пакет из строки
    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacket);

    // Проверяем корректность десериализованного пакета
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);
    BOOST_CHECK_EQUAL(deserializedPacket.component_kind(), componentKind);
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), payload);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
