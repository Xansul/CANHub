#include "tracemodel.h"
#include <algorithm>

TraceModel::TraceModel(QObject *parent)
    : QAbstractListModel{parent}
{
    m_clock.start();

    m_flushTimer.setInterval(kFlushIntervalMs);
    connect(&m_flushTimer, &QTimer::timeout, this, &TraceModel::flushPending);
    m_flushTimer.start();

    //placeholder for UI testing
    TraceEntry entry;
    m_entries.push_back(entry);
}

int TraceModel::rowCount(const QModelIndex &parent) const
{
    //only return data if top level index (no parent)
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(m_entries.size());
}

QVariant TraceModel::data(const QModelIndex &index, int role) const
{
    //safety checks
    if (!index.isValid() || index.row() < 0 || static_cast<size_t>(index.row()) >= m_entries.size())
    {
        //return empty QVariant
        return {};
    }

    //ref for performance
    const TraceEntry &entry = m_entries[static_cast<size_t>(index.row())];

    switch (role) {
    case TimestampRole:     return entry.timestampMs / 1000; //convert to seconds
    case CobIdRole:         return QStringLiteral("0x%1").arg(entry.cobId, 3, 16, QLatin1Char('0')).toUpper(); //convert to hex string
    case DirectionRole:     return entry.isTx ? QStringLiteral("TX") : QStringLiteral("RX");
    case DataHexRole:       return QString::fromLatin1(entry.payload.toHex(' ').toUpper());
    default:                return {};
    }
}

QHash<int, QByteArray> TraceModel::roleNames() const
{
    return {
        { TimestampRole,    "timestamp" },
        { CobIdRole,        "cobId" },
        { DirectionRole,    "direction" },
        { DataHexRole,      "dataHex" }
    };
}

void TraceModel::onFrameReceived(const QCanBusFrame &frame)
{
    enqueue(frame, false);
}

void TraceModel::onFrameSent(const QCanBusFrame &frame)
{
    enqueue(frame, true);
}

void TraceModel::enqueue(const QCanBusFrame &frame, bool isTx)
{
    TraceEntry entry;
    entry.timestampMs = m_clock.elapsed();
    entry.cobId = frame.frameId();
    entry.isTx = isTx;
    entry.payload = frame.payload();

    //don't touch model yet - handled by flushPending
    m_pending.push_back(entry);
}

void TraceModel::flushPending()
{
    if (m_pending.empty())
    {
        return;
    }

    const int pendingCount = static_cast<int>(m_pending.size());

    //insert newest first - reverse iterator
    beginInsertRows(QModelIndex(), 0, pendingCount - 1);
    for (auto it = m_pending.rbegin(); it != m_pending.rend(); ++it)
    {
        m_entries.push_front(*it);
    }
    endInsertRows();
    m_pending.clear();

    //calculate eviction count and remove
    if (m_entries.size() > static_cast<size_t>(kMaxEntries))
    {
        const size_t toRemove = m_entries.size() - static_cast<size_t>(kMaxEntries);

        const int firstRow = static_cast<int>(m_entries.size() - toRemove);
        const int lastRow = static_cast<int>(m_entries.size() - 1);

        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        m_entries.erase(m_entries.end() - static_cast<long>(toRemove), m_entries.end());
        endRemoveRows();
    }
}