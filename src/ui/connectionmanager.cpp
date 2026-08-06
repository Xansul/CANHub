#include "connectionmanager.h"
#include <QCanBus>
#include <QMetaObject>

ConnectionManager::ConnectionManager(BusEngine *busEngine, QObject *parent)
    : QObject{parent}, m_busEngine{busEngine}
{
    //auto upgrades connection type to queued
    connect(m_busEngine, &BusEngine::connectionStateChanged, this, &ConnectionManager::onConnectionStateChanged);
    connect(m_busEngine, &BusEngine::busError, this, &ConnectionManager::onBusError);
}

bool ConnectionManager::connected() const
{
    return m_connected;
}

QString ConnectionManager::statusMessage() const
{
    return m_statusMessage;
}

QStringList ConnectionManager::availablePlugins() const
{
    return {
        QStringLiteral("socketcan"),
        QStringLiteral("peakcan"),
        QStringLiteral("vectorcan"),
        QStringLiteral("tinycan"),
        QStringLiteral("systeccan"),
        QStringLiteral("virtualcan")
    };
}

QStringList ConnectionManager::availableInterfaces(const QString &pluginName) const
{
    //get devices for plugin name
    QStringList result;
    QString errorString;
    const QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(pluginName, &errorString);

    //fill in interface names
    for (const QCanBusDeviceInfo &info : devices)
    {
        result.append(info.name());
    }

    return result;
}

void ConnectionManager::connectTo(const QString &pluginName, const QString &interfaceName, int bitRate)
{
    //connection will be queued - give immediate update for UI
    setStatusMessage(QStringLiteral("Connecting..."));

    //FIFO queue
    QMetaObject::invokeMethod(m_busEngine, "configure", Qt::QueuedConnection, Q_ARG(QString, pluginName), Q_ARG(QString, interfaceName), Q_ARG(int, bitRate));
    QMetaObject::invokeMethod(m_busEngine, "connectDevice", Qt::QueuedConnection);
}

void ConnectionManager::disconnectBus()
{
    QMetaObject::invokeMethod(m_busEngine, "disconnectDevice", Qt::QueuedConnection);
}

QList<int> ConnectionManager::commonBitRates() const
{
    return { 10000, 20000, 50000, 100000, 125000, 250000, 500000, 800000, 1000000 };
}

void ConnectionManager::onConnectionStateChanged(bool isConnected)
{
    setConnected(isConnected);
    setStatusMessage(isConnected ? QStringLiteral("Connected") : QStringLiteral("Disconnected"));
}

void ConnectionManager::onBusError(const QString &message)
{
    setStatusMessage(message);
}

void ConnectionManager::setConnected(bool value)
{
    //check if same
    if (m_connected == value)
    {
        return;
    }

    m_connected = value;
    emit connectedChanged();
}

void ConnectionManager::setStatusMessage(const QString &message)
{
    //check if same
    if (m_statusMessage == message)
    {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}