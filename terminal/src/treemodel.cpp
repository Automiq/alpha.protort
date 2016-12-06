#include <QtWidgets>

#include "treemodel.h"
#include <QList>

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{

//    setupModelData(/*QList<RemoteNode>xml*/);
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
            return m_nodes[index.parent().row()].components()[index.row()].Name();
        case Input:
            return m_nodes[index.parent().row()].components()[index.row()].Input();
        case Output:
            return m_nodes[index.parent().row()].components()[index.row()].Output();
        default:
            return 0;
        }


    switch(index.column())
    {
    case Name:
        return m_nodes[index.row()].name();
    case Connect:
        return m_nodes[index.row()].connection();
    case Speed:
        return m_nodes[index.row()].speed();
    case Uptime:
        return m_nodes[index.row()].uptime();
    case Input:
        return m_nodes[index.row()].input();
    case Output:
        return m_nodes[index.row()].output();
    }
    //    writeStatusLog(tr("<Название узла - %1>").arg(QString::fromStdString(status.node_name())));
    //    writeStatusLog(tr("<Время работы - %2 сек.>").arg(QString::number(status.uptime())));
    //    writeStatusLog(tr("<Количество принятых пакетов - %3 (%4 байт)>")
    //                   .arg(QString::number(status.in_packets_count()))
    //                   .arg(QString::number(status.in_bytes_count())));
    //    writeStatusLog(tr("<Количество переданных пакетов - %3 (%4 байт)>")
    //                   .arg(QString::number(status.out_packets_count()))
    //                   .arg(QString::number(status.out_bytes_count())));
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


    // TODO

    return createIndex( parentItem->childNumber(), 0, parentItem);
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
