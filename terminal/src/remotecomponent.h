#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>

class RemoteNode;

class RemoteComponent : public QObject
{
    Q_OBJECT

    uint32_t packetsReceived_;
    uint32_t packetsSent_;
    QString name_;
    RemoteNode *parent_;

public:
    RemoteComponent(QObject *parent = Q_NULLPTR);

    QString name() const { return name_; }
    uint32_t packetsReceived() const { return packetsReceived_; }
    uint32_t packetsSent() const { return packetsSent_; }
    RemoteNode* parent(){ return parent_; }

    void setName(QString name){ name_ = name; }
    void setPacketsReceived(uint32_t value) { packetsReceived_ = value; }
    void setPacketsSent(uint32_t value) { packetsSent_ = value; }
    void setParent(RemoteNode* prnt){ parent_ = prnt; }
};

#endif // COMPONENT_H
