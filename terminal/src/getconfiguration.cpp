#include "getconfiguration.h"

GetConfiguration::GetConfiguration()
{

}

GetConfiguration::GetConfiguration(std::string ip_address, uint32_t port, uint32_t config_port)
{
    alpha::protort::parser::node info;
    info.host.config_port = config_port;
    info.host.ip_address = ip_address;
    info.host.port = port;
    info.name = "NodeTMP";

    Node = boost::make_shared<RemoteNode>(info);

}

void GetConfiguration::createNode(std::string ip_address, uint32_t port, uint32_t config_port)
{

    alpha::protort::parser::node info;
    info.host.config_port = config_port;
    info.host.ip_address = ip_address;
    info.host.port = port;
    info.name = "NodeTMP_";

    Node = boost::make_shared<RemoteNode>(info);


}

void GetConfiguration::connectionNode(boost::asio::io_service &service)
{
    Node->init(service);
}


void GetConfiguration::configurationRequest()
{
    alpha::protort::protocol::Packet_Payload config;
    config.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::GetConfig);
    Node->async_config(config);
}


void GetConfiguration::parserResponce(const alpha::protort::protocol::deploy::Packet& config)
{
    nodeInfo nodeConfig;

    auto conf = config.response().get_config().config();
    nodeConfig.node_name = conf.this_node_info().name();
    nodeConfig.addres = conf.this_node_info().address();
    nodeConfig.port = conf.this_node_info().port();
    nodeConfig.back_status = conf.this_node_info().backup_status();

    for (auto & inst : conf.instances())
        nodeConfig.node_data.components.push_back({inst.name(), get_component_kind(inst.kind())});

    for (auto & conn : conf.connections())
        nodeConfig.node_data.connections.push_back({conn.source().name(), conn.source().port(),
                                     conn.destination().name(), conn.destination().port()});

    for (auto & node : conf.node_infos())
    {
        if(nodeConfig.node_name == node.name())
            continue;
        nodeConfig.node_data.nodes.push_back({node.name(), node.address(), node.port()});
    }

    for (auto & map : conf.maps())
        nodeConfig.node_data.mappings.push_back({map.instance_name(), map.node_name()});


   configuration_system_.push_back(nodeConfig);

}

void GetConfiguration::outConfigSystem()
{
    QFile fileD("deploy.xml");
    if(fileD.open(QIODevice::WriteOnly | QIODevice::Text))
    {
          QTextStream write(&fileD);
          write <<"<deploy>" <<"\n";
          for(int i = 0; i < configuration_system_.size(); ++i)
               write << " <node name=\042" << QString::fromStdString(configuration_system_[i].node_name) << "\042 address=\042"
                     << QString::fromStdString(configuration_system_[i].addres) << "\042 port=\042" << QString::number(configuration_system_[i].port) << "\042 /> \n";

          for(int i = 0; i < configuration_system_.size(); ++i)
          {
               for(auto & map : configuration_system_[i].node_data.mappings )
               {
                   if (configuration_system_[i].node_name != map.node_name)
                        continue;
                   write << "<map instance=\042" << QString::fromStdString(map.comp_name)<<"\042 node=\042" << QString::fromStdString(map.node_name)<< "\042 /> \n";
               }
          }
          write <<"</deploy>" <<"\n";
        fileD.close();
    }

    QFile fileA("app.xml");
    if(fileA.open(QIODevice::WriteOnly | QIODevice::Text))
    {
          QTextStream write(&fileA);
          write <<"<app>" <<"\n";


          for(int i = 0; i < configuration_system_.size(); ++i)
          {
               for(auto & inst : configuration_system_[i].node_data.components )
                   write << " <instance name=\042" << QString::fromStdString(inst.name)<<"\042 kind=\042" << QString::fromStdString(inst.kind)<< "\042 /> \n";
          }

          for(int i = 0; i < configuration_system_.size(); ++i)
          {
               for(auto & conn : configuration_system_[i].node_data.connections )
                   write << " <connection source=\042" << QString::fromStdString(conn.source)<<"\042 source_out=\042" << QString::number(conn.source_out)<<
                            "\042 dest=\042" << QString::fromStdString(conn.dest)<<"\042 dest_in=\042" << QString::number(conn.dest_in)<< "\042 /> \n";
          }
          write <<"</app>" <<"\n";
        fileA.close();
    }



}

 std::string GetConfiguration::get_component_kind(const alpha::protort::protocol::ComponentKind& kind)
{
    switch (kind) {
    case alpha::protort::protocol::ComponentKind::Generator:
        return "generator";
    case alpha::protort::protocol::ComponentKind::Retranslator:
        return "retranslator";
    case alpha::protort::protocol::ComponentKind::Terminator:
        return "terminator";
    case alpha::protort::protocol::ComponentKind::Calc:
        return "calc";
    case alpha::protort::protocol::ComponentKind::History:
        return "history";
    case alpha::protort::protocol::ComponentKind::TimedGenerator:
        return "timed_generator";
    default:
        assert(false);
        return {};
    }
}
