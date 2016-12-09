#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

#include "parser.h"
#include "remotenode.h"

using RemoteNodePtr = boost::shared_ptr<RemoteNode>;
class RemoteComponent;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QList<RemoteNodePtr> &data,
              QObject *parent = 0);
    ~TreeModel();

    void setupModelData(const QList<RemoteNodePtr> &nodes);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private slots:
    void onComponentsChanged();
    void onStatusChanged();

private:    
    QList<RemoteNodePtr> m_nodes;

    QObject *object(const QModelIndex &index) const;
    RemoteNode *node(const QModelIndex &index) const;
    RemoteComponent *component(const QModelIndex &index) const;

    int indexOfNode(RemoteNode* node) const;

    RemoteNode *nodeAt(int index) const;

    QVariant nodeData(RemoteNode* node, const QModelIndex &index, int role) const;
    QVariant componentData(RemoteComponent* component, const QModelIndex &index, int role) const;

    enum Column
    {
        Name,
        Address,
        Connection,
        Uptime,
        PacketsReceived,
        BytesReceived,
        PacketsSent,
        BytesSent,
        LastColumn = BytesSent,
        ColumnCount = LastColumn + 1,
    };
};

#endif // TREEMODEL_H
