#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <vector>

#include "node.h"
#include "components.h"
#include "router.h"

namespace alpha {
namespace protort {
namespace node {
namespace tests {

void test_node_router()
{
    int retranslator_count = 0;
    int terminator_count = 0;

    std::vector<alpha::protort::components::component *> components;

    // Создаем объекты outport_ (1 outport_ - 1 выходной порт)

    router<node>::routes mas_output_ports[7];

    // Создаем объекты local_input

    router<node>::local_route mas_endpoint[7];

    // Создаем node_router
    boost::asio::io_service service;
    router<node> router_(service);

    // Создаем компоненты
    components::generator generator1_(router_);
    components.push_back(&generator1_);

    components::retranslator retranslator1_(router_);
    components.push_back(&retranslator1_);
    components::retranslator retranslator2_(router_);
    components.push_back(&retranslator2_);
    components::retranslator retranslator3_(router_);
    components.push_back(&retranslator3_);

    components::terminator terminator1_(router_);
    components.push_back(&terminator1_);
    components::terminator terminator2_(router_);
    components.push_back(&terminator2_);
    components::terminator terminator3_(router_);
    components.push_back(&terminator3_);

    // Присваиваем каждому объекту component_with_connections указатель на компонент и имя компонента

    for(int i = 0;i < 7;i++)
    {
        router<node>::component_instance component_instances;
        component_instances.component_ = components[i];

        if(typeid(*components[i]) == typeid(components::generator))
            component_instances.name = "g1";
        if(typeid(*components[i]) == typeid(components::terminator))
            component_instances.name = "t" + std::to_string(++terminator_count);
        if(typeid(*components[i]) == typeid(components::retranslator))
            component_instances.name = "r" + std::to_string(++retranslator_count);

        router_.components.insert(std::make_pair(component_instances.name,component_instances));
        components[i]->set_comp_inst(&router_.components[component_instances.name]);
    }

    // Определяем для каждого компонента выходные порты и соединения для них

    // g1
    router_.components["g1"].port_to_routes.insert(std::make_pair(0,mas_output_ports[0]));
    router_.components["g1"].port_to_routes.insert(std::make_pair(1,mas_output_ports[1]));

    // r1
    router_.components["r1"].port_to_routes.insert(std::make_pair(0,mas_output_ports[2]));
    router_.components["r1"].port_to_routes.insert(std::make_pair(1,mas_output_ports[3]));

    // r2
    router_.components["r2"].port_to_routes.insert(std::make_pair(0,mas_output_ports[4]));

    // r3
    router_.components["r3"].port_to_routes.insert(std::make_pair(0,mas_output_ports[5]));
    router_.components["r3"].port_to_routes.insert(std::make_pair(1,mas_output_ports[6]));

    // Определяем входные порты для соединений
    for(int i = 0;i < 7;i++)
        mas_endpoint[i].in_port = 0;

    // Определяем входные порты компонентов и
    // для каждого выходного порта добавляем входные порты других компонентов

    // g1 - r1 g1 - r3
    mas_endpoint[0].component = &router_.components["r1"];
    mas_endpoint[1].component = &router_.components["r3"];

    router_.components["g1"].port_to_routes[0].local_routes.push_back(mas_endpoint[0]);
    router_.components["g1"].port_to_routes[1].local_routes.push_back(mas_endpoint[1]);

    // r1 - r2 r1 - t2
    mas_endpoint[2].component = &router_.components["r2"];
    mas_endpoint[3].component = &router_.components["t2"];

    router_.components["r1"].port_to_routes[0].local_routes.push_back(mas_endpoint[2]);
    router_.components["r1"].port_to_routes[1].local_routes.push_back(mas_endpoint[3]);

    // r2 - t1
    mas_endpoint[4].component = &router_.components["t1"];

    router_.components["r2"].port_to_routes[0].local_routes.push_back(mas_endpoint[4]);

    // r3 - t2 r3 - t3
    mas_endpoint[5].component = &router_.components["t2"];
    mas_endpoint[6].component = &router_.components["t3"];

    router_.components["r3"].port_to_routes[0].local_routes.push_back(mas_endpoint[5]);
    router_.components["r3"].port_to_routes[0].local_routes.push_back(mas_endpoint[6]);


    router_.route("g1",0,"string for generator");
}

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
    test_node_router();
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
