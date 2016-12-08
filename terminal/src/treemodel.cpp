#include <QtWidgets>

#include "treemodel.h"
#include <QList>

TreeModel::TreeModel(const QList<RemoteNodePtr> &data, QObject *parent)
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
            return m_nodes[index.parent().row()].get()->components()[index.row()].name();
        case Input:
            return m_nodes[index.parent().row()].get()->components()[index.row()].input();
        case Output:
            return m_nodes[index.parent().row()].get()->components()[index.row()].output();
        default:
            return 0;
        }


    switch(index.column())
    {
    case Name:
        return m_nodes[index.row()].get()->name();
    case Connect:
        return m_nodes[index.row()].get()->isConnect();
    case Speed:
        return QString::number(m_nodes[index.row()].get()->speed());
    case Uptime:
        return m_nodes[index.row()].get()->uptime();
    case Input:
        return m_nodes[index.row()].get()->input().getPackets();
    case Output:
        return m_nodes[index.row()].get()->output().getPackets();
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

    return m_nodes[parent.row()].get()->components().size();
}

void TreeModel::setupModelData(const QList<RemoteNodePtr> &nodes)
{
    beginResetModel();
    m_nodes = nodes;
    endResetModel();
}

int TreeModel::findParent(int index) const
{
    int i = 0;
    int sum = m_nodes[i].get()->components().size();

    while (sum < index)
    {
        ++i;
        sum += m_nodes[i].get()->components().size() + 1;
    }

    if (i == 0)
        return 0;

    else
    {
        sum -= m_nodes[i].get()->components().size() + 1;
        --i;
        if (!i)
            return 0;

        return sum + 1;
    }
}

//void TreeModel::setNodes(const std::unordered_map<std::string, alpha::protort::parser::node> &n)
//{

//}

//void TreeModel::setComponents(const std::vector<alpha::protort::parser::mapping> &m)
//{
//    int size = m.size();
//    for (int i = 0; i < size; ++i)
//        findNAdd(QString::fromStdString(m[i].comp_name), QString::fromStdString(m[i].node_name));
//}

//void TreeModel::findNAdd(QString comp_name, QString node_name)
//{
//    int size = m_nodes.size();
//    for (int i = 0; i < size; ++i)
//        if (m_nodes[i].name() == node_name)
//        {
//            m_nodes[i].addComp(comp_name);
//            return;
//        }
//}
