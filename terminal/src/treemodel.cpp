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

RemoteNode *TreeModel::nodeAt(int index) const
{
    return m_nodes[index].get();
}

QVariant TreeModel::nodeData(RemoteNode *node, const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole && index.column() == Column::Connection)
        return QIcon(node->isConnected() ? ":/images/connected.png" : ":/images/notconnected.ico");

    if (role == Qt::DecorationRole && index.column() == Column::Swap && !node->pairNodeStatus())
        return QIcon(":/images/slave.png");

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column())
    {
    case Column::Name:
        return node->name();
    case Column::Address:
        return node->address();
    }

    if (!node->isConnected())
        return QVariant();

    switch(index.column())
    {
    case Column::Uptime:
        return node->uptime();
    case Column::PacketsReceived:
        return node->packetsReceived();
    case Column::BytesReceived:
        return humanReadableBytes(node->bytesReceived());
    case Column::DownSpeed:
        return humanReadableSpeed(node->downSpeed());
    case Column::PacketsSent:
        return node->packetsSent();
    case Column::BytesSent:
        return humanReadableBytes(node->bytesSent());
    case Column::UpSpeed:
        return humanReadableSpeed(node->upSpeed());
    default:
        return QVariant();
    }
}

QVariant TreeModel::componentData(RemoteComponent *component, const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch(index.column())
    {
    case Column::Name:
        return component->name();
    case Column::PacketsReceived:
        return component->packetsReceived();
    case Column::PacketsSent:
        return component->packetsSent();
    default:
        return QVariant();
    }
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::DecorationRole)
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
    case Column::Name:
        return tr("Имя узла");
    case Column::Swap:
        return tr("Состояние");
    case Column::Address:
        return tr("Адрес");
    case Column::Connection:
        return tr("Связь");
    case Column::Uptime:
        return tr("Время работы");
    case Column::PacketsReceived:
        return tr("Принято пакетов");
    case Column::BytesReceived:
        return tr("Принято");
    case Column::DownSpeed:
        return tr("Вх. скорость");
    case Column::PacketsSent:
        return tr("Отправлено пакетов");
    case Column::BytesSent:
        return tr("Отправлено");
    case Column::UpSpeed:
        return tr("Исх. скорость");
    default:
        return QVariant();
    }
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    if (m_nodes.isEmpty())
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, nodeAt(row));

    auto n = node(parent);
    Q_ASSERT(n);

    return createIndex(row, column, n->componentAt(row));
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (m_nodes.isEmpty())
        return QModelIndex();

    if (auto n = node(index))
        return QModelIndex();

    if (auto comp = component(index))
    {
        auto node = comp->parent();
        int idx = indexOfNode(node);
        if (node && idx != -1)
            return createIndex(idx, 0, node);
    }

    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_nodes.size();

    if (auto n = node(parent))
        return n->components().size();

    return 0;
}

void TreeModel::setupModelData(const QList<RemoteNodePtr> &nodes)
{
    beginResetModel();
    m_nodes = nodes;
    for (auto &node : m_nodes)
    {
        auto *n = node.get();
        /*! Здесь можно проверить состояние ноды
         * if(node.pairNodeStatus())
         * {
         *
         * }
         */
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

    auto idx = indexOfNode(node);

    auto parentLeft = index(idx, 0);
    auto parentRight = index(idx, Column::LastColumn);

    emit dataChanged(parentLeft, parentRight);

    if (!node->components().size())
        return;

    auto topLeft = index(0, 0, parentLeft);
    auto bottomRight = index(node->components().size() - 1, Column::LastColumn, parentLeft);

    emit dataChanged(topLeft, bottomRight);
}

QString TreeModel::humanReadableBytes(double bytes)
{
    QStringList list;
    list << "КиБ" << "МиБ" << "ГиБ" << "ТиБ";

    QStringListIterator i(list);
    QString unit("байт");

    while(bytes >= 1024.0 && i.hasNext())
    {
        unit = i.next();
        bytes /= 1024.0;
    }
    return QString().setNum(bytes, 'f', 2) + " " + unit;
}

QString TreeModel::humanReadableSpeed(double speed)
{
    return speed ? QString("%1/сек").arg(humanReadableBytes(speed)) : tr("N/A");
}
