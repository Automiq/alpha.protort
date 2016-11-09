#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "packet.pb.h"
#include "components.pb.h"
#include "deploy.pb.h"

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
    communication::Packet packet;
    packet.set_payload(payload);
    packet.mutable_source()->CopyFrom(sourceEndpoint);
    packet.mutable_destination()->CopyFrom(destEndpoint);

    // Проверяем корректность сформированного пакета
    BOOST_CHECK_EQUAL(packet.source().component_kind(), sourceEndpoint.component_kind());
    BOOST_CHECK_EQUAL(packet.source().port(), sourceEndpoint.port());
    BOOST_CHECK_EQUAL(packet.destination().component_kind(), destEndpoint.component_kind());
    BOOST_CHECK_EQUAL(packet.destination().port(), destEndpoint.port());
    BOOST_CHECK_EQUAL(packet.payload(), payload);

    // Сериализуем пакет в строку
    auto serializedPacket = packet.SerializeAsString();

    // Десериализуем пакет из строки
    communication::Packet deserializedPacket;
    deserializedPacket.ParseFromString(serializedPacket);

    // Проверяем корректность десериализованного пакета
    BOOST_CHECK_EQUAL(deserializedPacket.source().component_kind(), sourceEndpoint.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.source().port(), sourceEndpoint.port());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().component_kind(), destEndpoint.component_kind());
    BOOST_CHECK_EQUAL(deserializedPacket.destination().port(), destEndpoint.port());
    BOOST_CHECK_EQUAL(deserializedPacket.payload(), payload);
}

BOOST_AUTO_TEST_CASE(test_serialize_trerminal_to_node_protocol){
    //формируем тестовые данные
    const std::string node_name = "node";
    const std::string node_addr = "localhost";
    const uint32_t node_port = 100;

    const std::string terminator_name = "terminator";
    const ComponentKind terminator_kind = ComponentKind::Terminator;
    const uint32_t terminator_port_out = 1, terminator_port_in = 0;

    const std::string generator_name = "generator";
    const ComponentKind generator_kind = ComponentKind::Generator;
    const uint32_t generator_port_out = 1, generator_port_in = 0;

    //пакет с информацией о ноде
    deploy::NodeInfo info;
    info.set_name(node_name);
    info.set_address(node_addr);
    info.set_port(node_port);

    deploy::NodeInfoList info_list;
    info_list.add_node_info()->CopyFrom(info);

    BOOST_CHECK_EQUAL(info_list.node_info(0).name(),node_name);
    BOOST_CHECK_EQUAL(info_list.node_info(0).address(),node_addr);
    BOOST_CHECK_EQUAL(info_list.node_info(0).port(),node_port);

    deploy::Instance terminator, generator;
    terminator.set_kind(terminator_kind);
    terminator.set_name(terminator_name);
    generator.set_kind(generator_kind);
    generator.set_name(generator_name);

    deploy::InstanceList inst_list;
    inst_list.add_instance()->CopyFrom(terminator);
    inst_list.add_instance()->CopyFrom(generator);

    BOOST_CHECK_EQUAL(inst_list.instance(0).kind(),terminator_kind);
    BOOST_CHECK_EQUAL(inst_list.instance(0).name(),terminator_name);
    BOOST_CHECK_EQUAL(inst_list.instance(1).kind(),generator_kind);
    BOOST_CHECK_EQUAL(inst_list.instance(1).name(),generator_name);

    deploy::Map terminator_map, generator_map;
    terminator_map.set_node_name(node_name);
    terminator_map.set_instance_name(terminator_name);
    generator_map.set_node_name(node_name);
    generator_map.set_instance_name(generator_name);

    deploy::MapList map_list;
    map_list.add_map()->CopyFrom(terminator_map);
    map_list.add_map()->CopyFrom(generator_map);

    BOOST_CHECK_EQUAL(map_list.map(0).node_name(),node_name);
    BOOST_CHECK_EQUAL(map_list.map(0).instance_name(),terminator_name);
    BOOST_CHECK_EQUAL(map_list.map(1).node_name(),node_name);
    BOOST_CHECK_EQUAL(map_list.map(1).instance_name(),generator_name);

    ComponentEndpoint from_generator, to_terminator;
    from_generator.set_name(generator_name);
    from_generator.set_port(generator_port_out);
    to_terminator.set_name(terminator_name);
    to_terminator.set_port(terminator_port_in);

    deploy::Connection con;
    con.mutable_source()->CopyFrom(from_generator);
    con.mutable_destination()->CopyFrom(to_terminator);

    deploy::ConnectionList con_list;
    con_list.add_connection()->CopyFrom(con);

    BOOST_CHECK_EQUAL(con_list.connection(0).source().name(),generator_name);
    BOOST_CHECK_EQUAL(con_list.connection(0).source().port(),generator_port_out);
    BOOST_CHECK_EQUAL(con_list.connection(0).destination().name(),terminator_name);
    BOOST_CHECK_EQUAL(con_list.connection(0).destination().port(),terminator_port_in);

    auto serialized_info_list = info_list.SerializeAsString(),
         serialized_map_list  = map_list.SerializeAsString(),
         serialized_inst_list = inst_list.SerializeAsString(),
         serialized_con_list  = con_list.SerializeAsString();

    deploy::NodeInfoList deserialized_info_list;
    deserialized_info_list.ParseFromString(serialized_info_list);

    deploy::MapList deserialized_map_list;
    deserialized_map_list.ParseFromString(serialized_map_list);

    deploy::InstanceList deserialized_inst_list;
    deserialized_inst_list.ParseFromString(serialized_inst_list);

    deploy::ConnectionList deserialized_con_list;
    deserialized_con_list.ParseFromString(serialized_con_list);

    BOOST_CHECK_EQUAL(deserialized_info_list.node_info(0).name(),node_name);
    BOOST_CHECK_EQUAL(deserialized_info_list.node_info(0).address(),node_addr);
    BOOST_CHECK_EQUAL(deserialized_info_list.node_info(0).port(),node_port);

    BOOST_CHECK_EQUAL(deserialized_inst_list.instance(0).kind(),terminator_kind);
    BOOST_CHECK_EQUAL(deserialized_inst_list.instance(0).name(),terminator_name);
    BOOST_CHECK_EQUAL(deserialized_inst_list.instance(1).kind(),generator_kind);
    BOOST_CHECK_EQUAL(deserialized_inst_list.instance(1).name(),generator_name);

    BOOST_CHECK_EQUAL(deserialized_map_list.map(0).node_name(),node_name);
    BOOST_CHECK_EQUAL(deserialized_map_list.map(0).instance_name(),terminator_name);
    BOOST_CHECK_EQUAL(deserialized_map_list.map(1).node_name(),node_name);
    BOOST_CHECK_EQUAL(deserialized_map_list.map(1).instance_name(),generator_name);

    BOOST_CHECK_EQUAL(deserialized_con_list.connection(0).source().name(),generator_name);
    BOOST_CHECK_EQUAL(deserialized_con_list.connection(0).source().port(),generator_port_out);
    BOOST_CHECK_EQUAL(deserialized_con_list.connection(0).destination().name(),terminator_name);
    BOOST_CHECK_EQUAL(deserialized_con_list.connection(0).destination().port(),terminator_port_in);

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace protocol
} // namespace protort
} // namespace alpha
