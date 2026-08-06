#include "pdoconsolemodel.h"

PDOConsoleModel::PDOConsoleModel(PDOEngine *pdoEngine, NodeManager *nodeManager, QObject *parent)
    : QAbstractListModel{parent}, m_pdoEngine{pdoEngine}, m_nodeManager{nodeManager}
{
    connect(m_pdoEngine, &PDOEngine::PDOReceived, this, &PDOConsoleModel::onPDOReceived);
}

int PDOConsoleModel::rowCount(const QModelIndex &parent) const
{
    //only return data if top level index (no parent)
    if (parent.isValid())
    {
        return 0;
    }

    return m_PDOs.size();
}

QVariant PDOConsoleModel::data(const QModelIndex &index, int role) const
{
    //safety checks
    if (!index.isValid() || index.row() < 0 || index.row() >= m_PDOs.size())
    {
        return {};
    }

    const PDO &pdo = m_PDOs.at(index.row());

    switch (role) {
    case COBIdRole:         return QStringLiteral("0x%1").arg(pdo.cobId, 3, 16, QLatin1Char('0')).toUpper();
    case NodeIdRole:        return QString::number(pdo.nodeId);
    case DataHexRole:       return QString::fromLatin1(pdo.lastValue.toHex(' ').toUpper());
    default:                return {};
    }
}

QHash<int, QByteArray> PDOConsoleModel::roleNames() const
{
    return {
        { COBIdRole,    "cobId" },
        { NodeIdRole,   "nodeId" },
        { DataHexRole,  "dataHex" }
    };
}

void PDOConsoleModel::addSubscription(int cobId, int nodeId)
{
    //narrowing conversion
    const quint32 id = static_cast<quint32>(cobId);

    //safety check
    if (rowForCobId(id) != -1)
    {
        //already subscribed
        return;
    }

    PDO pdo;
    pdo.cobId = id;
    pdo.nodeId = static_cast<quint8>(nodeId);

    m_pdoEngine->subscribe(id);
    m_nodeManager->registerPDOCobId(pdo.nodeId, pdo.cobId);

    const int newRow = m_PDOs.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_PDOs.append(pdo);
    endInsertRows();
}

void PDOConsoleModel::removeSubscription(int cobId)
{
    //narrowing conversion
    const quint32 id = static_cast<quint32>(cobId);

    const int row = rowForCobId(id);

    //safety check
    if (row == -1)
    {
        //already gone
        return;
    }

    m_pdoEngine->unsubscribe(id);

    beginRemoveRows(QModelIndex(), row, row);
    m_PDOs.removeAt(row);
    endRemoveRows();
}

void PDOConsoleModel::sendPDO(int cobId, const QString &hexData)
{
    QByteArray cleaned = hexData.toUtf8();
    cleaned.replace(" ", "");
    const QByteArray data = QByteArray::fromHex(cleaned);

    //safety check - size check of 8 bytes
    if (data.isEmpty() || data.size() > 8)
    {
        return;
    }

    m_pdoEngine->sendPDO(static_cast<quint32>(cobId), data);
}

void PDOConsoleModel::pollPDO(int cobId)
{
    m_pdoEngine->requestPDO(static_cast<quint32>(cobId));
}

void PDOConsoleModel::onPDOReceived(quint32 cobId, const QByteArray &data)
{
    const int row = rowForCobId(cobId);

    if (row == -1)
    {
        //not actively tracking
        return;
    }

    m_PDOs[row].lastValue = data;

    const QModelIndex changedIndex = index(row);
    emit dataChanged(changedIndex, changedIndex);
}

int PDOConsoleModel::rowForCobId(quint32 cobId) const
{
    //manual scan - linear
    for (int i = 0; i < m_PDOs.size(); i++)
    {
        if (m_PDOs.at(i).cobId == cobId)
        {
            return i;
        }
    }

    //return -1 if not found
    return -1;
}