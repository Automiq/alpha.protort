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


    //Геттеры{
    //Возвращает имя компоненты
    QString name() const { return name_; }
    //Возвращает количество принятых пакетов
    uint32_t packetsReceived() const { return packetsReceived_; }
    //Возвращает количество отправленных пакетов
    uint32_t packetsSent() const { return packetsSent_; }
    //Возвращает указатель на родителя
    RemoteNode* parent(){ return parent_; }
    //}Геттеры

    //Сеттеры{
    //Задает имя компоненты
    void setName(QString name){ name_ = name; }
    //Задает количество принятых пакетов
    void setPacketsReceived(uint32_t value) { packetsReceived_ = value; }
    //Задает количество отправленных пакетов
    void setPacketsSent(uint32_t value) { packetsSent_ = value; }
    //Задает указатель на родителя
    void setParent(RemoteNode* prnt){ parent_ = prnt; }
    //}Сеттеры
};

#endif // COMPONENT_H
