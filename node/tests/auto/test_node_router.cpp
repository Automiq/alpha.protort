#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <vector>

#include "node.h"
#include "node_router.h"
#include "all_components.h"

namespace alpha {
namespace protort {
namespace node {
namespace tests {

struct fixture
{
    ~fixture()
    {
        google::protobuf::ShutdownProtobufLibrary();
    }
};

BOOST_FIXTURE_TEST_SUITE(tests_node_router,fixture)

BOOST_AUTO_TEST_CASE(test_node_router)
{
    int retranslator_count = 0;
    int terminator_count = 0;

    std::vector<alpha::protort::components::i_component *> components;

    // Создаем объекты outport_ (1 outport_ - 1 выходной порт)

    node_router<node>::outport_ mas_output_ports[7];

    // Создаем объекты local_input

    node_router<node>::local_input_ mas_endpoint[7];

    // Создаем node_router

    node_router<node> router_;

    // Создаем компоненты
    components::generator generator1_;
    components.push_back(&generator1_);

    components::retranslator retranslator1_;
    components.push_back(&retranslator1_);
    components::retranslator retranslator2_;
    components.push_back(&retranslator2_);
    components::retranslator retranslator3_;
    components.push_back(&retranslator3_);

    components::terminator terminator1_;
    components.push_back(&terminator1_);
    components::terminator terminator2_;
    components.push_back(&terminator2_);
    components::terminator terminator3_;
    components.push_back(&terminator3_);

    // Присваиваем каждому объекту component_with_connections указатель на компонент и имя компонента

    for(int i = 0;i < 7;i++)
    {
        node_router<node>::component_with_connections component_connections;
        component_connections.component_ = components[i];

        if(typeid(*components[i]) == typeid(components::generator))
            component_connections.name = "g1";
        if(typeid(*components[i]) == typeid(components::terminator))
            component_connections.name = "t" + std::to_string(++terminator_count);
        if(typeid(*components[i]) == typeid(components::retranslator))
            component_connections.name = "r" + std::to_string(++retranslator_count);

        router_.component_list.insert(std::make_pair(component_connections.name,component_connections));
    }

    // Определяем для каждого компонента входные порты и соединения для них

    // g1
    router_.component_list["g1"].map_all_connections.insert(std::make_pair(0,mas_output_ports[0]));
    router_.component_list["g1"].map_all_connections.insert(std::make_pair(1,mas_output_ports[1]));

    // r1
    router_.component_list["r1"].map_all_connections.insert(std::make_pair(0,mas_output_ports[2]));
    router_.component_list["r1"].map_all_connections.insert(std::make_pair(1,mas_output_ports[3]));

    // r2
    router_.component_list["r2"].map_all_connections.insert(std::make_pair(0,mas_output_ports[4]));

    // r3
    router_.component_list["r3"].map_all_connections.insert(std::make_pair(0,mas_output_ports[5]));
    router_.component_list["r3"].map_all_connections.insert(std::make_pair(1,mas_output_ports[6]));

    // Определяем входные порты для соединений
    for(int i = 0;i < 7;i++)
        mas_endpoint[i].port = 0;

    // Определяем входные порты компонентов и
    // для каждого выходного порта добавляем входные порты других компонентов

    // g1 - r1 g1 - r3
    mas_endpoint[0].connection = &router_.component_list["r1"];
    mas_endpoint[1].connection = &router_.component_list["r3"];

    router_.component_list["g1"].map_all_connections[0].local_components.push_back(mas_endpoint[0]);
    router_.component_list["g1"].map_all_connections[1].local_components.push_back(mas_endpoint[1]);

    // r1 - r2 r1 - t2
    mas_endpoint[2].connection = &router_.component_list["r2"];
    mas_endpoint[3].connection = &router_.component_list["t2"];

    router_.component_list["r1"].map_all_connections[0].local_components.push_back(mas_endpoint[2]);
    router_.component_list["r1"].map_all_connections[1].local_components.push_back(mas_endpoint[3]);

    // r2 - t1
    mas_endpoint[4].connection = &router_.component_list["t1"];

    router_.component_list["r2"].map_all_connections[0].local_components.push_back(mas_endpoint[4]);

    // r3 - t2 r3 - t3
    mas_endpoint[5].connection = &router_.component_list["t2"];
    mas_endpoint[6].connection = &router_.component_list["t3"];

    router_.component_list["r3"].map_all_connections[0].local_components.push_back(mas_endpoint[5]);
    router_.component_list["r3"].map_all_connections[0].local_components.push_back(mas_endpoint[6]);


    router_.do_process("g1",0,"string for generator");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
