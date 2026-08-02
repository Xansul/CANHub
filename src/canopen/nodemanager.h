#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <QObject>
#include <QHash>
#include <QList>
#include "NodeSession.h"

//forward declarations
class NMTMaster;
class HeartbeatMonitor;
class SDOClient;
class PDOEngine;

//Aggregates all canopen classes into one NodeSession per node - nodes are registered the first time we hear about a node id
class NodeManager : public QObject
{
    Q_OBJECT

public:
    explicit NodeManager(NMTMaster *nmtMaster, HeartbeatMonitor *heartbeatMonitor, SDOClient *sdoClient, PDOEngine *pdoEngine, QObject *parent = nullptr);

    QList<quint8> knownNodes() const;
    NodeSession session(quint8 nodeId) const;

    //register cobIds to nodes
    void registerPDOCobId(quint8 nodeId, quint32 cobId);

signals:
    void nodeAdded(quint8 nodeId);
    void nodeUpdated(quint8 nodeId);

private slots:
    //handlers for all external signals
    void onNMTStateChanged(quint8 nodeId, NodeState state);
    void onNodeTimedOut(quint8 nodeId);
    void onNodeRecovered(quint8 nodeId);
    void onSDOUploadFinished(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data);
    void onSDODownloadFinished(quint8 nodeId, quint16 index, quint8 subIndex);
    void onSDOAborted(quint8 nodeId, quint16 index, quint8 subIndex, quint32 abortCode);
    void onPDOReceived(quint32 cobId, const QByteArray &data);

private:
    //used to start tracking new nodes
    void ensureNode(quint8 nodeId);

    //non-owning pointers
    NMTMaster *m_nmtMaster;
    HeartbeatMonitor *m_heartbeatMonitor;
    SDOClient *m_sdoClient;
    PDOEngine *m_pdoEngine;

    QHash<quint8, NodeSession> m_sessions;
    QHash<quint32, quint8> m_PDOCobIdToNode;

    //placeholder for heartbeat periods
    static constexpr int kDefaultHeartbeatTimeoutMs = 1000;
};

#endif // NODEMANAGER_H
