#ifndef GETCONFIGURATION_H
#define GETCONFIGURATION_H

#include <boost/make_shared.hpp>
#include "remotenode.h"
#include "client.h"
#include "parser.h"

#include <QFile>
#include <QTextStream>
class GetConfiguration
{
public:
    GetConfiguration();

    GetConfiguration(std::string ip_address, uint32_t port, uint32_t config_port);
    //метод для подключения к ноде
    void connectionNode(boost::asio::io_service &service);
    //метод для отправки запроса конфигуриции
    void configurationRequest();
    //парсит и сохраняет информацию о нодах
    void parserResponce(const alpha::protort::protocol::deploy::Packet& config);
    //метод для создания фиктивной ноды.
    void createNode(std::string ip_address, uint32_t port, uint32_t config_port);
    //преобразование типа
     std::string get_component_kind(const alpha::protort::protocol::ComponentKind& kind);
    // вывод конфигурации
    void outConfigSystem();
     // финктивная нода, необходима для подключения и отправки запроса
     boost::shared_ptr<RemoteNode> Node;

private:

    struct nodeInfo
    {
        std::string node_name;
        std::string addres;
        uint32_t port;
        alpha::protort::protocol::backup::BackupStatus back_status;

        alpha::protort::parser::configuration node_data;
    };


    QVector<nodeInfo> configuration_system_;
};

#endif // GETCONFIGURATION_H
