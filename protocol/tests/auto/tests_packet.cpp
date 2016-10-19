#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "packet.pb.h"
#include "iostream"

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
    const std::string payload = "hello_worm";

    ComponentEndpoint sourceComponent;
    sourceComponent.set_component_kind(ComponentKind::Generator);
    sourceComponent.set_port(2);
	
    ComponentEndpoint destComponent;
    destComponent.set_component_kind(ComponentKind::Retranslator);
    destComponent.set_port(2);

    // Формируем пакет
    Packet packet;
    packet.set_id(1);
    packet.set_payload(payload);

    packet.mutable_source()->set_component_kind(sourceComponent);
    packet.set_allocated_destination(&destComponent);

    // Проверяем корректность сформированного пакета
    BOOST_CHECK_EQUAL(packet.id(), id);
    BOOST_CHECK_EQUAL(packet.source().component_kind(), sourceComponent.component_kind());
    BOOST_CHECK_EQUAL(packet.source().port(), sourceComponent.port());
    BOOST_CHECK_EQUAL(packet.destination().component_kind(), destComponent.component_kind());
    BOOST_CHECK_EQUAL(packet.destination().port(), destComponent.port());
    BOOST_CHECK_EQUAL(packet.payload(), payload);

    // Сериализуем пакет в строку
    auto serializedPacket = packet.SerializeAsString();

    // Десериализуем пакет из строки
    Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacket);

    // Проверяем корректность десериализованного пакета
    BOOST_CHECK_EQUAL(deserializedPacket.id(), id);
    BOOST_CHECK_EQUAL(deserializedPacket.source().component_kind(), sourceComponent.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.source().port(), sourceComponent.port());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().component_kind(), destComponent.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().port(), destComponent.port());
    std::cout<<payload;
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), payload);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
