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

using namespace alpha::protort;

Q_DECLARE_METATYPE(protocol::Packet_Payload)
Q_DECLARE_METATYPE(protocol::deploy::Packet)
Q_DECLARE_METATYPE(protocol::deploy::StatusResponse)
Q_DECLARE_METATYPE(boost::system::error_code)

class RemoteComponent;

enum class BackupStatus
{
    None = 0,
    Master= 1,
    Slave = 2
};

class RemoteNode : public QObject, public boost::enable_shared_from_this<RemoteNode>
{
    Q_OBJECT

    using client_t = alpha::protort::protolink::client<RemoteNode>;
    using client_ptr = boost::shared_ptr<client_t>;
    using RemoteNodePtr = boost::shared_ptr<RemoteNode>;

public:   
    QList<RemoteComponent*> components() const;

    RemoteNode(const parser::node &node,
               protocol::backup::BackupStatus backup_status);
    ~RemoteNode();

    void init(boost::asio::io_service &service);
    void init_pairnode(boost::shared_ptr<RemoteNode> &pairnode);
    void shutdown();

    QString name() const;
    QString address() const;
    QString info() const;
    boost::weak_ptr<RemoteNode> pairnode() const;

    void init_info_node(const std::string &name_node,
                        const parser::address &address,
                        protocol::backup::BackupStatus backup_status,
                        protocol::deploy::NodeInfo &node_info) const;

    RemoteNode &search_pairnode(const std::string &name_node,
                                const QList<RemoteNodePtr> &remote_nodes) const;

    void async_backup_transition(alpha::protort::protocol::Packet_Payload& backup);
    void async_deploy(deploy_configuration &deploy_configuration_, const QList<RemoteNodePtr> &remote_node);
    void async_start(protocol::Packet_Payload &packet);
    void async_stop(protocol::Packet_Payload &packet);
    void async_status(protocol::Packet_Payload &status);



    //! Методы получения данных узла
    bool isConnected() const;
    uint32_t uptime() const;
    uint32_t packetsReceived() const;
    uint32_t packetsSent() const;

    uint32_t bytesReceived() const;
    uint32_t bytesSent() const;

    uint32_t downSpeed() const;
    uint32_t upSpeed() const;

    BackupStatus backupStatus() const;


    RemoteComponent *componentAt(int index) const;

signals:
    void deployConfigRequestFinished(const protocol::deploy::Packet &packet);
    void statusRequestFinished(const protocol::deploy::Packet &packet);
    void startRequestFinished(const protocol::deploy::Packet &packet);
    void stopRequestFinished(const protocol::deploy::Packet &packet);
    void backupTransitionRequestFinished(const alpha::protort::protocol::deploy::Packet&);
    void connected();
    void connectionFailed(const boost::system::error_code &err);

    void componentsChanged();
    void statusChanged();

private slots:
    void onStatusRequestFinished(const protocol::deploy::Packet &packet);
    void onConnected();
    void onConnectionFailed(const boost::system::error_code &err);

private:

    friend class alpha::protort::protolink::client<RemoteNode>;

    //! Колбеки для protolink::client
    //@{
    void on_connected(const boost::system::error_code &err);
    void on_packet_sent(const boost::system::error_code &err, size_t bytes);
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
    void setBackupStatus(alpha::protort::protocol::backup::BackupStatus value);

    double calcUpSpeed(const QTime &now, uint32_t bytesSent);
    double calcDownSpeed(const QTime &now, uint32_t bytesReceived);
    double calcSpeed(const QTime &now, uint32_t lastBytes, uint32_t nowBytes);

    //! Информация об узле, с которым коннектится клиент
    parser::node node_information_;
    boost::weak_ptr<RemoteNode> pairnode_;
    protocol::backup::BackupStatus backup_status_;

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

    QString name_;
    QList<RemoteComponent*> components_;

    QTime m_lastStatusTime;

    BackupStatus backupStatus_;
};
#endif // TERMINAL_CLIENT_H
