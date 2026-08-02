#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include "src/transport/busengine.h"
#include "src/canopen/sdoclient.h"
#include "src/canopen/nmtmaster.h"
#include "src/canopen/pdoengine.h"
#include "src/canopen/heartbeatmonitor.h"
#include "src/canopen/nodemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //create objects
    auto *busThread = new QThread;
    //no parent - will be moved to thread
    auto *busEngine = new BusEngine;
    //stays on main thread
    auto *sdoClient = new SDOClient;
    auto *nmtMaster = new NMTMaster;
    auto *pdoEngine = new PDOEngine;
    auto *heartbeatMonitor = new HeartbeatMonitor;
    auto *nodeManager = new NodeManager(nmtMaster, heartbeatMonitor, sdoClient, pdoEngine);

    //move to thread - busEngine slots will now run on busThread
    busEngine->moveToThread(busThread);

    //wire up deletion
    QObject::connect(busThread, &QThread::finished, busEngine, &QObject::deleteLater);

    //start thread
    busThread->start();

    //cross thread wiring - uses AutoConnection to upgrade to queued connections
    QObject::connect(busEngine, &BusEngine::frameReceived, sdoClient, &SDOClient::onFrameReceived, Qt::AutoConnection);
    QObject::connect(sdoClient, &SDOClient::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, nmtMaster, &NMTMaster::onFrameReceived, Qt::AutoConnection);
    QObject::connect(nmtMaster, &NMTMaster::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, pdoEngine, &PDOEngine::onFrameReceived, Qt::AutoConnection);
    QObject::connect(pdoEngine, &PDOEngine::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, heartbeatMonitor, &HeartbeatMonitor::onFrameReceived, Qt::AutoConnection);

    //temp hardcoded adapter selection
#if defined(Q_OS_WIN)
    busEngine->configure("peakcam", "usb0");
#elif defined(Q_OS_LINUX)
    busEngine->configure("socketcan", "can0");
#else
    busEngine->configure("virtualcan", "vcan0");
#endif

    //queue device connection
    QMetaObject::invokeMethod(busEngine, "connectDevice", Qt::QueuedConnection);

    QQmlApplicationEngine engine;

    engine.loadFromModule("CANHub", "Main");

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }

    const int result = app.exec();

    //shutdown threads
    busThread->quit();
    busThread->wait();

    return result;
}
