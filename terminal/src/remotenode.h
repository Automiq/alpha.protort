﻿#ifndef TERMINAL_CLIENT_H
#define TERMINAL_CLIENT_H

#include <iostream>
#include <QList>
#include <QString>
#include <QMetaType>
#include <QObject>
#include <set>
#include <boost/make_shared.hpp>

#include "client.h"
#include "parser.h"
#include "ui_mainwindow.h"
#include "deployconfiguration.h"


Q_DECLARE_METATYPE(alpha::protort::protocol::Packet_Payload);
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::StatusResponse);
Q_DECLARE_METATYPE(boost::system::error_code);

class RemoteNode : public QObject, public boost::enable_shared_from_this<RemoteNode>
{
    Q_OBJECT

    using client_t = alpha::protort::protolink::client<RemoteNode>;
    using client_ptr = boost::shared_ptr<client_t>;

private:
    /*!
     * \brief Информация о компоненте
     * \param name - название компоненты
     * \param info - список состояний компонента
     * \param node - указатель на узел, на котором размещена данная компонента
     */
    struct Component
    {
        QString name_;
        uint32_t input_;
        uint32_t output_;
        RemoteNode *node_;
    };

public:
    QList<Component> components();

    RemoteNode(alpha::protort::parser::node const& node);

    void init(boost::asio::io_service& service);

    void shutdown();

    QString name() const;
    QString address() const;
    QString info() const;

    void async_deploy(deploy_configuration& deploy_configuration_);
    void async_start(alpha::protort::protocol::Packet_Payload& packet);
    void async_stop(alpha::protort::protocol::Packet_Payload& packet);
    void async_status(alpha::protort::protocol::Packet_Payload& status);


    //! Методы изменения данных узла
    void setName(QString name);
    void setConnect(uint32_t con);
    void setUptime(uint32_t time);
    void setSpeed(uint32_t speed);
    void setOutput(uint32_t packets, uint32_t bytes);
    void setInput(uint32_t packets, uint32_t bytes);

    //! Методы изменения данных компоненты
    void setCompName(const int &index, QString name);
    void setCompInput(const int &index, uint32_t packets);
    void setCompOutput(const int &index, uint32_t packets);
//    void setCompNode(const int &index, );

    //! Методы получения данных узла
    QString name();
    bool isConnect();
    uint32_t uptime();
    uint32_t speed();
    uint32_t output();
    uint32_t input();

    //! Методы получения данных компоненты
    QString compName(const int &index);
    uint32_t compOutput(const int &index);
    uint32_t compInput(const int &index);

signals:
    void deployConfigRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void statusRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void startRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void stopRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void connected();
    void connectionFailed(const boost::system::error_code&);

private:

    friend class alpha::protort::protolink::client<RemoteNode>;

    //! Колбеки для protolink::client
    //@{
    void on_connected(const boost::system::error_code& err);
    void on_packet_sent(const boost::system::error_code& err, size_t bytes);
    void on_new_packet(alpha::protort::protocol::Packet_Payload packet);
    //@}

    //! Информация об узле, с которым коннектится клиент
    alpha::protort::parser::node node_information_;

    //! Клиент для подключения к узлу
    client_ptr client_;

    QString name_;
    uint32_t uptime_;
    uint32_t speed_;
    QList<Component> components_;
};

#endif // TERMINAL_CLIENT_H
