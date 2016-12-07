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
    class Packet
    {
        uint32_t bytes_;
        uint32_t packets_;

    public:
        Packet(){ packets_ = 0; bytes_ = 0; }
        Packet(const Packet &pac){ packets_ = pac.packets_, bytes_ = pac.bytes_; }
        Packet operator =(const Packet &pac){ return Packet(pac.packets_, pac.bytes_); }
        Packet(uint32_t p, uint32_t b){ packets_ = p; bytes_ = b; }
        void setBytes(uint32_t b){ bytes_ = b; }
        void setPackets(uint32_t p){ packets_ = p; }
        uint32_t getBytes(){ return bytes_; }
        uint32_t getPackets(){ return packets_; }
        void clear();
        Packet operator()(uint32_t p, uint32_t b);
    };

    class Component
    {
        uint32_t input_;
        uint32_t output_;
        
        QString name_;
        RemoteNode *parent_;

    public:
        uint32_t input() const { return input_; }
        QString name() const { return name_; }
        uint32_t output() const { return output_; }
        RemoteNode* parent(){ return parent_; }

        void setInput(uint32_t packets){ input_ = packets; }
        void setName(QString name){ name_ = name; }
        void setOutput(uint32_t packets){ output_ = packets; }
        void setParent(RemoteNode* p){ parent_ = p; }
        Component(const Component &c)
        {
            input_ = c.input_;
            output_ = c.output_;
            name_ = c.name_;
            parent_ = c.parent_;
        }

        Component operator =(const Component &c){ return Component(c); }
    };

public:
    QList<Component> components() const;
    Component operator [](int index) const;

    RemoteNode(alpha::protort::parser::node const& node);
    RemoteNode(const RemoteNode &n);
    RemoteNode operator =(const RemoteNode &n);

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
    void setId(int id);

    //! Методы получения данных узла
    QString name();
    bool isConnect() const;
    uint32_t uptime() const ;
    uint32_t speed() const;
    Packet output() const ;
    Packet input() const ;
    QModelIndex index() const;
    int id() const;

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

    uint32_t uptime_;
    uint32_t speed_;

    Packet in_;
    Packet out_;

    bool connection_;
    int id_;

    QString name_;
    QList<Component> components_;
    QModelIndex index_;
};
#endif // TERMINAL_CLIENT_H
