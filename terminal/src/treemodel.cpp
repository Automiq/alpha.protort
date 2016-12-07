#include <QtWidgets>

#include "treemodel.h"
#include <QList>

TreeModel::TreeModel(const QList<RemoteNode> &data, QObject *parent)
    : QAbstractItemModel(parent)
{

    setupModelData(data);
}

TreeModel::~TreeModel(){}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return Column::MaxColumn;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    //проверка на столбец
    if(index.parent().isValid())
        switch(index.column())
        {
        case Name:
            return m_nodes[index.parent().row()].components()[index.row()].name();
        case Input:
            return m_nodes[index.parent().row()].components()[index.row()].input();
        case Output:
            return m_nodes[index.parent().row()].components()[index.row()].output();
        default:
            return 0;
        }


    switch(index.column())
    {
    case Name:
        return m_nodes[index.row()].name();
    case Connect:
        return m_nodes[index.row()].isConnect();
    case Speed:
        return QString::number(m_nodes[index.row()].speed());
    case Uptime:
        return m_nodes[index.row()].uptime();
    case Input:
        return m_nodes[index.row()].input().getPackets();
    case Output:
        return m_nodes[index.row()].output().getPackets();
    }
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
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

        }
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > Column::MaxColumn)
        return QModelIndex();

//    void *internal = nullptr;

    if (parent.isValid())
//        internal = findParent(1);

    return createIndex(row, column);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    int parentRow = findParent(index.row());
        return createIndex(parentRow, 0);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_nodes.size();

    return m_nodes[parent.row()].components().size();
}

void TreeModel::setupModelData(const QList<RemoteNode> nodes)
{
    m_nodes = nodes;
}

int TreeModel::findParent(int index) const
{
    int i = 0;
    int sum = m_nodes[i].components().size();

    while (sum < index)
    {
        ++i;
        sum += m_nodes[i].components().size() + 1;
    }

    if (i == 0)
        return 0;

    else
    {
        sum -= m_nodes[i].components().size() + 1;
        --i;
        if (!i)
            return 0;

        return sum + 1;
    }
}
