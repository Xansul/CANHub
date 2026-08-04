#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include <QAbstractListModel>
#include <QTimer>
#include <QElapsedTimer>
#include <QCanBusFrame>
#include <deque>
#include <vector>
#include "TraceEntry.h"

//Ring buffer trace log - flushed into UI in batches on fixed timer - NOT on frame arrival - better performance
class TraceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TimestampRole = Qt::UserRole + 1,
        CobIdRole,
        DirectionRole,
        DataHexRole
    };

    explicit TraceModel(QObject *parent = nullptr);

    //overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void onFrameReceived(const QCanBusFrame &frame);
    void onFrameSent(const QCanBusFrame &frame);

private slots:
    void flushPending();

private:
    void enqueue(const QCanBusFrame &frame, bool isTx);

    std::deque<TraceEntry> m_entries;
    std::vector<TraceEntry> m_pending; //vector since only append and clear are needed
    QTimer m_flushTimer;
    QElapsedTimer m_clock;

    static constexpr int kMaxEntries = 5000;
    static constexpr int kFlushIntervalMs = 33; //30Hz
};

#endif // TRACEMODEL_H
