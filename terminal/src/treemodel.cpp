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

//Возвращает количество колонок
int TreeModel::columnCount(const QModelIndex &) const
{
    return Column::ColumnCount;
}

//Возвращает объект Qt по индексу
QObject* TreeModel::object(const QModelIndex &index) const
{
    return static_cast<QObject *>(index.internalPointer());
}

//Возвращает RemoteNode по индексу
RemoteNode *TreeModel::node(const QModelIndex &index) const
{
    return qobject_cast<RemoteNode *>(object(index));
}

//Возвращает RemoteComponent по индексу
RemoteComponent *TreeModel::component(const QModelIndex &index) const
{
    return qobject_cast<RemoteComponent *>(object(index));
}

//Возвращает индекс определенной RemoteNode. Параметр - указатель на RemoteNode
int TreeModel::indexOfNode(RemoteNode* node) const
{
    for (int i = 0, size = m_nodes.size(); i < size; ++i)
    {
        if (m_nodes[i].get() == node)
            return i;
    }
    return -1;
}

//Возвращает RemoteNode по индексу
RemoteNode *TreeModel::nodeAt(int index) const
{
    return m_nodes[index].get();
}

//Возвращает определенную инфу о remotenode. Например её иконку, имя, адрес, аптайм и т.д.
//Параметры: 1-укзатель на ноду, 2-индекс колонки(по сути-какую инфу хотим), 3-сложнааа..Похоже на.. декоративность элемента???
QVariant TreeModel::nodeData(RemoteNode *node, const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole && index.column() == Column::Connection)
        return QIcon(node->isConnected() ? ":/images/connected.png" : ":/images/notconnected.ico");

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

//Возвращает определенную инфу о КомпонентеНоды. Например его имя, кол-во отпр и полученных пакетов.
//Параметры: 1-указатель на компонент, 2-индекс колонки, 3-сложнааа..Похоже на.. декоративность элемента???
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

//Возвращает компонент или ноду индексу
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

//Позвращает имена колонки из заголовочной строки таблицы
//Основной параметр section - id столбца.
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch(section)
    {
    case Column::Name:
        return tr("Имя узла");
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

//Возвращение индекса RemoteComponent по строке, колонке, индексу родителя (RemoteNode)
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

//Возвращает индекс родителя компонента по индексу копмонента. 
//(Родитель - RemoteNode, компонент - RemoteComponent)
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

//Возвращает количество строк (количество RemoteComponent) у RemoteNode по индексу RemoteNode
int TreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_nodes.size();

    if (auto n = node(parent))
        return n->components().size();

    return 0;
}

//Привязка событий смены статуса и смены компонентов модели (RemoteNode`ов) к TreeModel
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

//Обновление компонентов RemoteNode в таблице TreeModel
void TreeModel::onComponentsChanged()
{
    auto node = qobject_cast<RemoteNode*>(sender());

    auto parent = index(indexOfNode(node), 0);

    emit dataChanged(parent, parent);
}

//Обновление статуса RemoteNode в таблице TreeModel
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

//Перевод байт в человекочитаемый размер. Возвращает строку
//Параметр 1 - количество байт.
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

//Перевод скорости в человекочитаемую скорость.
//Параметр 1 - скорость в байт/сек
QString TreeModel::humanReadableSpeed(double speed)
{
    return speed ? QString("%1/сек").arg(humanReadableBytes(speed)) : tr("N/A");
}
