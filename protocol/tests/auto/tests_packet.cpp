#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "packet.pb.h"
#include "components.pb.h"
#include "terminal-to-node.pb.h"

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
    const std::string payload = "payload";

    ComponentEndpoint sourceEndpoint;
    sourceEndpoint.set_component_kind(ComponentKind::Generator);
    sourceEndpoint.set_port(2);

    ComponentEndpoint destEndpoint;
    destEndpoint.set_component_kind(ComponentKind::Retranslator);
    destEndpoint.set_port(2);

    // Формируем пакет
    Packet packet;
    packet.set_id(1);
    packet.set_payload(payload);
    packet.mutable_source()->CopyFrom(sourceEndpoint);
    packet.mutable_destination()->CopyFrom(destEndpoint);

    // Проверяем корректность сформированного пакета
    BOOST_CHECK_EQUAL(packet.id(), id);
    BOOST_CHECK_EQUAL(packet.source().component_kind(), sourceEndpoint.component_kind());
    BOOST_CHECK_EQUAL(packet.source().port(), sourceEndpoint.port());
    BOOST_CHECK_EQUAL(packet.destination().component_kind(), destEndpoint.component_kind());
    BOOST_CHECK_EQUAL(packet.destination().port(), destEndpoint.port());
    BOOST_CHECK_EQUAL(packet.payload(), payload);

    // Сериализуем пакет в строку
    auto serializedPacket = packet.SerializeAsString();

    // Десериализуем пакет из строки
    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacket);

    // Проверяем корректность десериализованного пакета
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);
    BOOST_CHECK_EQUAL(deserializedPacket.source().component_kind(), sourceEndpoint.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.source().port(), sourceEndpoint.port());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().component_kind(), destEndpoint.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().port(), destEndpoint.port());
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), payload);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
