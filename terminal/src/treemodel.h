#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>

#include "parser.h"
#include "remotenode.h"

using RemoteNodePtr = boost::shared_ptr<RemoteNode>;

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

//    void setNodes(const std::unordered_map<std::__cxx11::string, alpha::protort::parser::node> &n);
//    void setComponents(const std::vector<alpha::protort::parser::mapping> &m);
//    void findNAdd(QString comp_name, QString node_name);

private:
    QList<RemoteNodePtr> m_nodes;

    int findParent(int index) const;

    enum Column
    {
        Name,
        Connect,
        Speed,
        Uptime,
        Input,
        Output,


        MaxColumn = Output + 1,
    };
};

#endif // TREEMODEL_H
