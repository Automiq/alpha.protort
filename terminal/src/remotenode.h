#ifndef TERMINAL_CLIENT_H
#define TERMINAL_CLIENT_H

#include <iostream>
#include <QMetaType>
#include <QObject>
#include <set>
#include <boost/make_shared.hpp>

#include "client.h"
#include "parser.h"
#include "ui_mainwindow.h"
#include "deploy_configuration.h"


Q_DECLARE_METATYPE(alpha::protort::protocol::Packet_Payload);
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::StatusResponse);
Q_DECLARE_METATYPE(boost::system::error_code);

class RemoteNode : public QObject, public boost::enable_shared_from_this<RemoteNode>
{
    Q_OBJECT

    using client_t = alpha::protort::protolink::client<RemoteNode>;
    using client_ptr = boost::shared_ptr<client_t>;

public:
    //! Информация об узле, с которым коннектится клиент
    alpha::protort::parser::node node_information_;

    RemoteNode(alpha::protort::parser::node const& node);

    void init(boost::asio::io_service& service);

    void shutdown();

    void async_deploy(deploy_configuration& deploy_configuration_);

    void async_start(alpha::protort::protocol::Packet_Payload& packet);

    void async_stop(alpha::protort::protocol::Packet_Payload& packet);

    void async_status(alpha::protort::protocol::Packet_Payload& status);

    void on_connected(const boost::system::error_code& err);

    void on_packet_sent(const boost::system::error_code& err, size_t bytes);

    void on_new_packet(alpha::protort::protocol::Packet_Payload packet);

signals:
    void deployConfigRequestFinished();
    void statusRequestFinished(alpha::protort::protocol::deploy::Packet const& status_);
    void startRequestFinished();
    void stopRequestFinished();
    void connected();
    void connectionFailed(const boost::system::error_code&);

private:

    client_ptr client_;
};

#endif // TERMINAL_CLIENT_H
