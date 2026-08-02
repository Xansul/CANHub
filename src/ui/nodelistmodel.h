#ifndef NODELISTMODEL_H
#define NODELISTMODEL_H

#include <QAbstractListModel>
#include "src/canopen/nodemanager.h"

//Model only tracks row order/nodeId - data calls read live through NodeManager
class NodeListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit NodeListModel(NodeManager *nodeManager, QObject *parent = nullptr);

    enum Roles {
        NodeIdRole = Qt::UserRole + 1,
        NMTStateRole,
        AliveRole
    };

    //model overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void onNodeAdded(quint8 nodeId);
    void onNodeUpdated(quint8 nodeId);

private:
    int rowForNodeId(quint8 nodeId) const;

    NodeManager *m_nodeManager;
    QList<quint8> m_nodeIds;
};

#endif // NODELISTMODEL_H
