#ifndef TERMINAL_CLIENT_H
#define TERMINAL_CLIENT_H

#include <iostream>
#include <set>
#include <boost/make_shared.hpp>

#include <QList>
#include <QString>
#include <QMetaType>
#include <QObject>
#include <QTime>

#include "client.h"
#include "parser.h"
#include "ui_mainwindow.h"
#include "deployconfiguration.h"


Q_DECLARE_METATYPE(alpha::protort::protocol::Packet_Payload)
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::Packet)
Q_DECLARE_METATYPE(alpha::protort::protocol::deploy::StatusResponse)
Q_DECLARE_METATYPE(boost::system::error_code)

class RemoteComponent;

class RemoteNode : public QObject, public boost::enable_shared_from_this<RemoteNode>
{
    Q_OBJECT

    using client_t = alpha::protort::protolink::client<RemoteNode>;
    using client_ptr = boost::shared_ptr<client_t>;

public:   

     void setBackupStatus(uint32_t value);
     void setBackupPushButtonStatus(bool status);

    QList<RemoteComponent*> components() const;

    RemoteNode(alpha::protort::parser::node const& node);
    ~RemoteNode();
    void init(boost::asio::io_service& service);

    void shutdown();

    QString name() const;
    QString address() const;
    QString info() const;

    void async_backup_transition(alpha::protort::protocol::Packet_Payload& backup);
    void async_deploy(deploy_configuration& deploy_configuration_);
    void async_start(alpha::protort::protocol::Packet_Payload& packet);
    void async_stop(alpha::protort::protocol::Packet_Payload& packet);
    void async_status(alpha::protort::protocol::Packet_Payload& status);



    //! Методы получения данных узла
    bool isConnected() const;
    uint32_t uptime() const;
    uint32_t packetsReceived() const;
    uint32_t packetsSent() const;

    uint32_t bytesReceived() const;
    uint32_t bytesSent() const;

    uint32_t downSpeed() const;
    uint32_t upSpeed() const;

    uint32_t backupStatus() const;
    bool bakupPushButtonStatus() const;

    RemoteComponent *componentAt(int index) const;

signals:
    void deployConfigRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void statusRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void startRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void stopRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void backupTransitionRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void connected();
    void connectionFailed(const boost::system::error_code&);

    void componentsChanged();
    void statusChanged();

private slots:
    void onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet&packet);
    void onConnected();
    void onConnectionFailed(const boost::system::error_code&);

private:

    friend class alpha::protort::protolink::client<RemoteNode>;

    //! Колбеки для protolink::client
    //@{
    void on_connected(const boost::system::error_code& err);
    void on_packet_sent(const boost::system::error_code& err, size_t bytes);
    void on_new_packet(alpha::protort::protocol::Packet_Payload packet);
    //@}

    void appendComponent(RemoteComponent *component);

    //! Методы изменения данных узла
    void setName(const QString &name);
    void setUptime(uint32_t time);
    void setPacketsReceived(uint32_t value);
    void setPacketsSent(uint32_t value);
    void setBytesReceived(uint32_t value);
    void setBytesSent(uint32_t value);
    void setConnected(bool value);
//    void setBackupStatus(uint32_t value);

    double calcUpSpeed(const QTime &now, uint32_t bytesSent);
    double calcDownSpeed(const QTime &now, uint32_t bytesReceived);
    double calcSpeed(const QTime &now, uint32_t lastBytes, uint32_t nowBytes);

    //! Информация об узле, с которым коннектится клиент
    alpha::protort::parser::node node_information_;

    //! Клиент для подключения к узлу
    client_ptr client_;

    uint32_t uptime_;

    uint32_t packetsReceived_;
    uint32_t bytesReceived_;

    uint32_t packetsSent_;
    uint32_t bytesSent_;

    double downSpeed_;
    double upSpeed_;

    bool isConnected_;
    bool backupButtonIsCreated_;

    QString name_;
    QList<RemoteComponent*> components_;

    QTime m_lastStatusTime;

//    enum BackupStatus
//    {
//        MasterButtonCtreated = 0,
//        SlaveButtonCtreated = 1,
//        NoneButtonCtreated = 2,
//        MasterButtonNone = 3,
//        SlaveButtonNone = 4,
//        NoneButtonNode = 5
//    } backupStatus_;
    uint32_t backupStatus_;
};
#endif // TERMINAL_CLIENT_H
