#ifndef HEARTBEATMONITOR_H
#define HEARTBEATMONITOR_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QCanBusFrame>

//Tracks heartbeat for nodes - no OD parsing - timeout is supplied by caller
class HeartbeatMonitor : public QObject
{
    Q_OBJECT

public:
    explicit HeartbeatMonitor(QObject *parent = nullptr);

    void watchNode(quint8 nodeId, int timeoutMs);
    void unwatchNode(quint8 nodeId);

    bool isAlive(quint8 nodeId) const;

public slots:
    void onFrameReceived(const QCanBusFrame &frame);

signals:
    void nodeTimedOut(quint8 nodeId);
    void nodeRecovered(quint8 nodeId);

private:
    QHash<quint8, QTimer*> m_timers;
    QSet<quint8> m_timedOutNodes;
};

#endif // HEARTBEATMONITOR_H
