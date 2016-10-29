#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/program_options.hpp>
#include <vector>

#include "node_router.h"
#include "all_components.h"

namespace alpha {
namespace protort {
namespace node {
namespace tests {

BOOST_AUTO_TEST_SUITE(tests_node_router)

BOOST_AUTO_TEST_CASE(test_node_router)
{
    /*
    const int component_amount = 7;
    int retranslator_count = 0;
    int terminator_count = 0;

    std::vector<alpha::protort::components::i_component *> components;

    // Создаем компоненты
    alpha::protort::components::generator generator1_;
    components.push_back(&generator1_);

    alpha::protort::components::retranslator retranslator1_;
    components.push_back(&retranslator1_);
    alpha::protort::components::retranslator retranslator2_;
    components.push_back(&retranslator2_);
    alpha::protort::components::retranslator retranslator3_;
    components.push_back(&retranslator3_);

    alpha::protort::components::terminator terminator1_;
    components.push_back(&terminator1_);
    alpha::protort::components::terminator terminator2_;
    components.push_back(&terminator2_);
    alpha::protort::components::terminator terminator3_;
    components.push_back(&terminator3_);

    // Создаем node_router
    node_router router_;

    // Создаем объекты outport_connections для каждого компонента
    node_router::outport_connections mas_out_con[component_amount];

    // Присваиваем каждому объекту outport_connections указатель на компонент и имя компонента

    for(int i = 0;i < component_amount;i++)
    {
        mas_out_con[i].component_ = component[i];
        if(typeid(component[i]).name() == "generator")
            mas_out_con[i].name = "g1";
        if(typeid(component[i]).name() == "terminator")
            mas_out_con[i].name = "t" + static_cast<std::string>(terminator_count++);
        if(typeid(component[i]).name() == "retranslator")
            mas_out_con[i].name = "t" + static_cast<std::string>(retranslator_count++);
    }

    // Создаем объекты connections (1 connections - 1 выходной порт)

    node_router::connections mas_output_ports[6];

    // Создаем объекты local_input, где указываем входной порт другого компонента и указатель на другой компонент
    */
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace node
} // namespace protort
} // namespace alpha
