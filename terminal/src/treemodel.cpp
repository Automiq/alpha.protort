#include <QtWidgets>
#include <QList>

#include "treemodel.h"
#include "remotecomponent.h"

TreeModel::TreeModel(const QList<RemoteNodePtr> &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    setupModelData(data);
}

TreeModel::~TreeModel(){}

int TreeModel::columnCount(const QModelIndex &) const
{
    return Column::ColumnCount;
}

QObject* TreeModel::object(const QModelIndex &index) const
{
    return static_cast<QObject *>(index.internalPointer());
}

RemoteNode *TreeModel::node(const QModelIndex &index) const
{
    return qobject_cast<RemoteNode *>(object(index));
}

RemoteComponent *TreeModel::component(const QModelIndex &index) const
{
    return qobject_cast<RemoteComponent *>(object(index));
}

int TreeModel::indexOfNode(RemoteNode* node) const
{
    for (int i = 0, size = m_nodes.size(); i < size; ++i)
    {
        if (m_nodes[i].get() == node)
            return i;
    }
    return -1;
}

QVariant TreeModel::nodeData(RemoteNode *node, const QModelIndex &index, int role) const
{
    switch(index.column())
    {
    case Name:
        return node->name();
    case Connect:
        return node->isConnect();
    case Speed:
        return QString::number(node->speed());
    case Uptime:
        return node->uptime();
    case Input:
        return node->input().getPackets();
    case Output:
        return node->output().getPackets();
    default:
        return QVariant();
    }
}

QVariant TreeModel::componentData(RemoteComponent *component, const QModelIndex &index, int role) const
{
    switch(index.column())
    {
    case Name:
        return component->name();
    case Input:
        return component->input();
    case Output:
        return component->output();
    default:
        return QVariant();
    }

}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (auto n = node(index))
        return nodeData(n, index, role);

    if (auto c = component(index))
        return componentData(c, index, role);

    return QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch(section)
    {
    case Name:
        return tr("Имя узла");
    case Connect:
        return tr("Связь");
    case Uptime:
        return tr("Время работы");
    case Input:
        return tr("Принято (пак./байт)");
    case Output:
        return tr("Отправлено (пак./байт)");
    case Speed:
        return tr("Скорость");
    default:
        return QVariant();
    }
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, 0, m_nodes[row].get());

    auto n = node(parent);
    Q_ASSERT(n);

    return createIndex(row, 0, n->components()[row]);

//    if (auto n = node(parent))
//        return createIndex(row, column, n)

//    Component* internal = &m_nodes[parent.row()].get()->components()[row];
//    if (parent.isValid())
//        return createIndex(row, column, internal);
//    return createIndex(row, column, m_nodes[row].get());
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (auto n = node(index))
        return QModelIndex();

    if (auto comp = component(index))
    {
        auto node = comp->parent();

        int size = m_nodes.size();
        int idx = indexOfNode(node);
        if (idx != -1)
            return createIndex(idx, 0, node);
    }

    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_nodes.size();

    return node(parent)->components().size();

//    return m_nodes[parent.row()].get()->components().size();
}

void TreeModel::setupModelData(const QList<RemoteNodePtr> &nodes)
{
    beginResetModel();
    m_nodes = nodes;
    for (auto &node : m_nodes)
    {
        auto *n = node.get();
        connect(n, &RemoteNode::componentsChanged, this, &TreeModel::onComponentsChanged);
        connect(n, &RemoteNode::statusChanged, this, &TreeModel::onStatusChanged);
    }

    endResetModel();
}

void TreeModel::onComponentsChanged()
{
    auto node = qobject_cast<RemoteNode*>(sender());

    auto parent = index(indexOfNode(node), 0);

    emit dataChanged(parent, parent);
}

void TreeModel::onStatusChanged()
{
    auto node = qobject_cast<RemoteNode*>(sender());

    auto parent = index(indexOfNode(node), 0);

    auto topLeft = index(0, 0, parent);
    auto bottomRight = index(node->components().size() - 1, Column::LastColumn, parent);

    emit dataChanged(topLeft, bottomRight);
}
