#ifndef TERMINAL_CLIENT_H
#define TERMINAL_CLIENT_H

#include <iostream>
#include <QMetaType>
#include <QObject>
#include <set>

#include "client.h"
#include "parser.h"
#include "ui_mainwindow.h"
#include "deploy_configuration.h"


Q_DECLARE_METATYPE(alpha::protort::protocol::Packet_Payload);
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::StatusResponse);

class RemoteNode : public QObject, public boost::enable_shared_from_this<RemoteNode>
{
    Q_OBJECT

public:
    RemoteNode(boost::asio::io_service& service, QString node_name);

    void async_deploy();
    void async_start();
    void async_stop();
    void async_status();

    alpha::protort::parser::deploy_configuration* deploy_configuration;

    boost::shared_ptr<alpha::protort::protolink::client<RemoteNode>> client_;

    void start_node(const alpha::protort::protocol::Packet_Payload& p);

    void stop_node(const alpha::protort::protocol::Packet_Payload& p);

    void status_node(const alpha::protort::protocol::Packet_Payload& p);

    void on_connected(const boost::system::error_code& err);

    void on_packet_sent(const boost::system::error_code& err, size_t bytes);

    void on_new_packet(alpha::protort::protocol::Packet_Payload packet_);

signals:
    void connected();
    void connectionFailed();

private slots:

    void on_status_response(alpha::protort::protocol::deploy::StatusResponse status_);

    void on_connected_finished(alpha::protort::protocol::Packet_Payload packet_);

    void on_start_finished(alpha::protort::protocol::Packet_Payload packet_);

    void on_stop_finished(alpha::protort::protocol::Packet_Payload packet_);

private:

    //! Имя узла, с которым коннектится клиент
    QString node_name_;
};

#endif // TERMINAL_CLIENT_H
