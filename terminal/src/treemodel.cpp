#include <QtWidgets>

#include "treeitem.h"
#include "treemodel.h"
#include <QList>

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{

    setupModelData(m_nodesList);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return column::maxcolumn;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

     //проверка на столбец
    if(index.parent().isValid())
        return m_nodes[index.parent().row()].components()[index.row()];

    return m_nodes[index.row()].name();
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

    void *internal = nullptr;

    if (parent.isValid())
        internal = m_nodes[parent.row()].get();

    return createIndex(row, column, internal);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();




    return createIndex( parentItem->childNumber(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_nodes.size();

    return m_nodes[parent.row()].components().size();
}

void TreeModel::setupModelData(const QList<RemoteNodePtr> nodes)
{
    m_nodes = nodes;
}
