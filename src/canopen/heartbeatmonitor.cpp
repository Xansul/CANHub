#include "heartbeatmonitor.h"

//internal vars - CiA 301 standard
namespace {
constexpr quint32 kHearbeatCobIdBase = 0x700; //same range as NMTMaster
}

HeartbeatMonitor::HeartbeatMonitor(QObject *parent)
    : QObject{parent}
{}

void HeartbeatMonitor::watchNode(quint8 nodeId, int timeoutMs)
{
    //delete timer if already there
    if (QTimer *existing = m_timers.value(nodeId))
    {
        existing->deleteLater();
    }

    //create timer - parented to this object for auto cleanup
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);

    //connect to lambda
    connect(timer, &QTimer::timeout, this, [this, nodeId] { m_timedOutNodes.insert(nodeId); emit nodeTimedOut(nodeId); });

    timer->start(timeoutMs);

    m_timers[nodeId] = timer;

    //assume alive until proven otherwise
    m_timedOutNodes.remove(nodeId);
}

void HeartbeatMonitor::unwatchNode(quint8 nodeId)
{
    if (QTimer *timer = m_timers.take(nodeId))
    {
        timer->stop();
        timer->deleteLater();
    }

    m_timedOutNodes.remove(nodeId);
}

bool HeartbeatMonitor::isAlive(quint8 nodeId) const
{
    if (!m_timers.contains(nodeId))
    {
        return false;
    }

    return !m_timedOutNodes.contains(nodeId);
}

void HeartbeatMonitor::onFrameReceived(const QCanBusFrame &frame)
{
    const quint32 id = frame.frameId();

    //range check
    if (id < kHearbeatCobIdBase || id > kHearbeatCobIdBase + 0x7F)
    {
        return;
    }

    const quint8 nodeId = static_cast<quint8>(id - kHearbeatCobIdBase);

    //check if we're watching the node
    QTimer *timer = m_timers.value(nodeId);
    if (!timer)
    {
        return;
    }

    //restart timer - no arguments restarts with previous interval
    timer->start();

    //check if recovered node
    if (m_timedOutNodes.remove(nodeId))
    {
        emit nodeRecovered(nodeId);
    }
}