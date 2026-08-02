#include "busengine.h"
#include <QCanBus>

BusEngine::BusEngine(QObject *parent)
    : QObject{parent}
{}

bool BusEngine::configure(const QString &pluginName, const QString &interfaceName)
{
    QString errorString;

    //create device
    m_device.reset(QCanBus::instance()->createDevice(pluginName, interfaceName, &errorString));

    if (!m_device)
    {
        emit busError(QStringLiteral("Failed to create device: %1").arg(errorString));
        return false;
    }

    //hook up slots
    connect(m_device.get(), &QCanBusDevice::framesReceived, this, &BusEngine::onFramesAvailable);
    connect(m_device.get(), &QCanBusDevice::errorOccurred, this, &BusEngine::onDeviceError);

    return true;
}

void BusEngine::connectDevice()
{
    if (!m_device)
    {
        emit busError(QStringLiteral("No device configured"));
        return;
    }

    //attempt connection
    if (m_device->connectDevice())
    {
        emit connectionStateChanged(true);
    }
    else
    {
        emit busError(m_device->errorString());
        emit connectionStateChanged(false);
    }
}

void BusEngine::disconnectDevice()
{
    if (m_device)
    {
        m_device->disconnectDevice();
        emit connectionStateChanged(false);
    }
}

void BusEngine::sendFrame(const QCanBusFrame &frame)
{
    if (!m_device || (m_device->state() != QCanBusDevice::ConnectedState))
    {
        emit busError(QStringLiteral("Cannot send: device not connected"));
        return;
    }

    m_device->writeFrame(frame);
}

void BusEngine::onFramesAvailable()
{
    //loop through available frames
    while (m_device->framesAvailable())
    {
        const QCanBusFrame frame = m_device->readFrame();
        emit frameReceived(frame);
    }
}

void BusEngine::onDeviceError(QCanBusDevice::CanBusError error)
{
    emit busError(QStringLiteral("Device error: %1, CAN error: %2").arg(m_device->errorString()).arg(error));

    //check if error caused disconnect
    if (m_device->ConnectedState == QCanBusDevice::UnconnectedState)
    {
        emit connectionStateChanged(false);
    }
}