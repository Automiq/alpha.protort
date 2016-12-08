#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>

class RemoteNode;

class RemoteComponent : public QObject
{
    Q_OBJECT

    uint32_t input_;
    uint32_t output_;
    QString name_;
    RemoteNode *parent_;

public:
    RemoteComponent(QObject *parent = Q_NULLPTR);

    uint32_t input() const { return input_; }
    QString name() const { return name_; }
    uint32_t output() const { return output_; }
    RemoteNode* parent(){ return parent_; }

    void setInput(uint32_t packets){ input_ = packets; }
    void setName(QString name){ name_ = name; }
    void setOutput(uint32_t packets){ output_ = packets; }
    void setParent(RemoteNode* prnt){ parent_ = prnt; }
};

#endif // COMPONENT_H
