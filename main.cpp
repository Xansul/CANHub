#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QQmlContext>
#include "src/transport/busengine.h"
#include "src/canopen/sdoclient.h"
#include "src/canopen/nmtmaster.h"
#include "src/canopen/pdoengine.h"
#include "src/canopen/heartbeatmonitor.h"
#include "src/canopen/nodemanager.h"
#include "src/ui/nodelistmodel.h"
#include "src/ui/tracemodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //create objects
    auto *busThread = new QThread(&app);
    //no parent - will be moved to thread
    auto *busEngine = new BusEngine;
    //stays on main thread
    auto *sdoClient = new SDOClient(&app);
    auto *nmtMaster = new NMTMaster(&app);
    auto *pdoEngine = new PDOEngine(&app);
    auto *heartbeatMonitor = new HeartbeatMonitor(&app);
    auto *nodeManager = new NodeManager(nmtMaster, heartbeatMonitor, sdoClient, pdoEngine, &app);
    auto *nodeListModel = new NodeListModel(nodeManager, &app);
    auto *traceModel = new TraceModel(&app);

    //move to thread - busEngine slots will now run on busThread
    busEngine->moveToThread(busThread);

    //wire up deletion
    QObject::connect(busThread, &QThread::finished, busEngine, &QObject::deleteLater);

    //start thread
    busThread->start();

    //cross thread wiring - uses AutoConnection to upgrade to queued connections
    QObject::connect(busEngine, &BusEngine::frameReceived, sdoClient, &SDOClient::onFrameReceived, Qt::AutoConnection);
    QObject::connect(sdoClient, &SDOClient::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(sdoClient, &SDOClient::frameToSend, traceModel, &TraceModel::onFrameSent, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, nmtMaster, &NMTMaster::onFrameReceived, Qt::AutoConnection);
    QObject::connect(nmtMaster, &NMTMaster::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(nmtMaster, &NMTMaster::frameToSend, traceModel, &TraceModel::onFrameSent, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, pdoEngine, &PDOEngine::onFrameReceived, Qt::AutoConnection);
    QObject::connect(pdoEngine, &PDOEngine::frameToSend, busEngine, &BusEngine::sendFrame, Qt::AutoConnection);
    QObject::connect(pdoEngine, &PDOEngine::frameToSend, traceModel, &TraceModel::onFrameSent, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, heartbeatMonitor, &HeartbeatMonitor::onFrameReceived, Qt::AutoConnection);
    QObject::connect(busEngine, &BusEngine::frameReceived, traceModel, &TraceModel::onFrameReceived, Qt::AutoConnection);

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

    //QML isn't creating instances - context property is appropriate here
    engine.rootContext()->setContextProperty("nodeListModel", nodeListModel);
    engine.rootContext()->setContextProperty("traceModel", traceModel);

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
