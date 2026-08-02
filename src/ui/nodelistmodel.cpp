#include "nodelistmodel.h"

NodeListModel::NodeListModel(NodeManager *nodeManager, QObject *parent)
    : QAbstractListModel{parent}, m_nodeManager{nodeManager}
{
    connect(m_nodeManager, &NodeManager::nodeAdded, this, &NodeListModel::onNodeAdded);
    connect(m_nodeManager, &NodeManager::nodeUpdated, this, &NodeListModel::onNodeUpdated);

    //fill any previously present nodes
    m_nodeIds = m_nodeManager->knownNodes();
}

int NodeListModel::rowCount(const QModelIndex &parent) const
{
    //only return data if top level index (no parent)
    if (parent.isValid())
    {
        return 0;
    }

    return m_nodeIds.size();
}

QVariant NodeListModel::data(const QModelIndex &index, int role) const
{
    //safety checks
    if (!index.isValid() || index.row() < 0 || index.row() > m_nodeIds.size())
    {
        return {};
    }

    //get nodeId
    const quint8 nodeId = m_nodeIds.at(index.row());
    const NodeSession session = m_nodeManager->session(nodeId);

    switch (role) {
    case NodeIdRole:            return session.nodeId;
    case NMTStateRole:          return static_cast<int>(session.NMTState);
    case AliveRole:             return session.alive;
    default:                    return {};
    }
}

QHash<int, QByteArray> NodeListModel::roleNames() const
{
    //inline build
    return {
        { NodeIdRole,   "nodeId" },
        { NMTStateRole, "NMTState" },
        { AliveRole,    "alive" }
    };
}

void NodeListModel::onNodeAdded(quint8 nodeId)
{
    //make sure node doesn't already exist
    if (rowForNodeId(nodeId) != -1)
    {
        return;
    }

    //get index from current size
    const int newRow = m_nodeIds.size();

    //add single row
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_nodeIds.append(nodeId);
    endInsertRows();
}

void NodeListModel::onNodeUpdated(quint8 nodeId)
{
    const int row = rowForNodeId(nodeId);
    if (row == -1)
    {
        //row isn't tracked
        return;
    }

    //update single row
    const QModelIndex changedIndex = index(row);
    emit dataChanged(changedIndex, changedIndex);
}

int NodeListModel::rowForNodeId(quint8 nodeId) const
{
    //O(n) lookup
    return m_nodeIds.indexOf(nodeId);
}







