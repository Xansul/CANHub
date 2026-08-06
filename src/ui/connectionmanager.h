#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QStringList>
#include "src/transport/busengine.h"

//UI facing bridge for CAN connection config - all calls into BusEngine go through a queued connection
class ConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit ConnectionManager(BusEngine *busEngine, QObject *parent = nullptr);

    bool connected() const;
    QString statusMessage() const;

    //plugin options - QCanBus has no way to list avaialable plugins, so this is hardcoded - not all will actually be present on a machine
    Q_INVOKABLE QStringList availablePlugins() const;

    //actual available interfaces for a given plugin - via QCanBus::availableDevices()
    Q_INVOKABLE QStringList availableInterfaces(const QString &pluginName) const;

    Q_INVOKABLE void connectTo(const QString &pluginName, const QString &interfaceName, int bitRate);
    Q_INVOKABLE void disconnectBus();

    //returns standard CiA 301 bit rates
    Q_INVOKABLE QList<int> commonBitRates() const;

signals:
    void connectedChanged();
    void statusMessageChanged();

private slots:
    void onConnectionStateChanged(bool isConnected);
    void onBusError(const QString &message);

private:
    void setConnected(bool value);
    void setStatusMessage(const QString &text);

    BusEngine *m_busEngine;
    bool m_connected = false;
    QString m_statusMessage;
};

#endif // CONNECTIONMANAGER_H
