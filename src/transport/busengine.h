#ifndef BUSENGINE_H
#define BUSENGINE_H

#include <QObject>
#include <QCanBusFrame>
#include <QCanBusDevice>

//Owns single CAN connection - lives on own thread - knows nothing about CANopen
class BusEngine : public QObject
{
    Q_OBJECT
public:
    explicit BusEngine(QObject *parent = nullptr);

public slots:
    //use slots to enable cross-thread queueing
    void connectDevice();
    void disconnectDevice();
    void sendFrame(const QCanBusFrame &frame);

    //configures connection - public slot so that it can be called from another thread via queued connection - bitrate is in bits/s - must be set before connectDevice()
    void configure(const QString &pluginName, const QString &interfaceName, int bitRate);

signals:
    void frameReceived(const QCanBusFrame &frame);
    void connectionStateChanged(bool connected);
    void busError(const QString &message);

private slots:
    void onFramesAvailable();
    void onDeviceError(QCanBusDevice::CanBusError error);

private:
    std::unique_ptr<QCanBusDevice> m_device;
};

#endif // BUSENGINE_H
