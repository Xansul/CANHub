#include "pdoengine.h"

PDOEngine::PDOEngine(QObject *parent)
    : QObject{parent}
{}

void PDOEngine::subscribe(quint32 cobId)
{
    m_subscriptions.insert(cobId);
}

void PDOEngine::unsubscribe(quint32 cobId)
{
    m_subscriptions.remove(cobId);
    m_lastValues.remove(cobId);
}

QByteArray PDOEngine::lastValue(quint32 cobId) const
{
    return m_lastValues.value(cobId);
}

void PDOEngine::sendPDO(quint32 cobId, const QByteArray &data)
{
    //no header format needed vs SDOs
    emit frameToSend(QCanBusFrame(cobId, data));
}

void PDOEngine::requestPDO(quint32 cobId)
{
    QCanBusFrame frame(cobId, QByteArray());
    frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    emit frameToSend(frame);
}

void PDOEngine::onFrameReceived(const QCanBusFrame &frame)
{
    //check for data frame
    if (frame.frameType() != QCanBusFrame::DataFrame)
    {
        return;
    }

    const quint32 cobId = frame.frameId();
    if (!m_subscriptions.contains(cobId))
    {
        //not subscribed
        return;
    }

    //overwrite last value for cobId
    m_lastValues[cobId] = frame.payload();
    emit PDOReceived(cobId, frame.payload());
}