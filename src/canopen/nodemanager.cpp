#include "nodemanager.h"
#include "nodemanager.h"
#include "nmtmaster.h"
#include "heartbeatmonitor.h"
#include "sdoclient.h"
#include "pdoengine.h"

NodeManager::NodeManager(NMTMaster *nmtMaster, HeartbeatMonitor *heartbeatMonitor, SDOClient *sdoClient, PDOEngine *pdoEngine, QObject *parent)
    : QObject{parent}, m_nmtMaster{nmtMaster}, m_heartbeatMonitor{heartbeatMonitor}, m_sdoClient{sdoClient}, m_pdoEngine{pdoEngine}
{
    //wire up all connections
    connect(m_nmtMaster, &NMTMaster::nodeStateChanged, this, &NodeManager::onNMTStateChanged);
    connect(m_heartbeatMonitor, &HeartbeatMonitor::nodeTimedOut, this, &NodeManager::onNodeTimedOut);
    connect(m_heartbeatMonitor, &HeartbeatMonitor::nodeRecovered, this, &NodeManager::onNodeRecovered);
    connect(m_sdoClient, &SDOClient::uploadFinished, this, &NodeManager::onSDOUploadFinished);
    connect(m_sdoClient, &SDOClient::downloadFinished, this, &NodeManager::onSDODownloadFinished);
    connect(m_sdoClient, &SDOClient::transactionAborted, this, &NodeManager::onSDOAborted);
    connect(m_pdoEngine, &PDOEngine::PDOReceived, this, &NodeManager::onPDOReceived);

    //placeholder for UI testing
    NodeSession session;
    m_sessions[0] = session;
}

QList<quint8> NodeManager::knownNodes() const
{
    return m_sessions.keys();
}

NodeSession NodeManager::session(quint8 nodeId) const
{
    //returns default session if nodeId isn't known
    return m_sessions.value(nodeId);
}

void NodeManager::ensureNode(quint8 nodeId)
{
    //do nothing if node already exists
    if (m_sessions.contains(nodeId))
    {
        return;
    }

    //create and track new NodeSession
    NodeSession newSession;
    newSession.nodeId = nodeId;
    newSession.alive = true;

    m_sessions[nodeId] = newSession;

    m_heartbeatMonitor->watchNode(nodeId, kDefaultHeartbeatTimeoutMs);

    emit nodeAdded(nodeId);
}

void NodeManager::onNMTStateChanged(quint8 nodeId, NodeState state)
{
    ensureNode(nodeId);

    m_sessions[nodeId].NMTState = state;

    emit nodeUpdated(nodeId);
}

void NodeManager::onNodeTimedOut(quint8 nodeId)
{
    if (!m_sessions.contains(nodeId))
    {
        return;
    }

    m_sessions[nodeId].alive = false;

    emit nodeUpdated(nodeId);
}

void NodeManager::onNodeRecovered(quint8 nodeId)
{
    if (!m_sessions.contains(nodeId))
    {
        return;
    }

    m_sessions[nodeId].alive = true;

    emit nodeUpdated(nodeId);
}

void NodeManager::registerPDOCobId(quint8 nodeId, quint32 cobId)
{
    ensureNode(nodeId);

    m_PDOCobIdToNode[cobId] = nodeId;

    m_pdoEngine->subscribe(cobId);
}

void NodeManager::onSDOUploadFinished(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data)
{
    ensureNode(nodeId);

    NodeSession &session = m_sessions[nodeId];
    session.lastSDOIndex = index;
    session.lastSDOSubIndex = subIndex;
    session.lastSDOValue = data;
    session.lastSDOAbortCode = 0; //reset abort code

    emit nodeUpdated(nodeId);
}

void NodeManager::onSDODownloadFinished(quint8 nodeId, quint16 index, quint8 subIndex)
{
    ensureNode(nodeId);

    NodeSession &session = m_sessions[nodeId];
    session.lastSDOIndex = index;
    session.lastSDOSubIndex = subIndex;
    session.lastSDOAbortCode = 0;

    emit nodeUpdated(nodeId);
}

void NodeManager::onSDOAborted(quint8 nodeId, quint16 index, quint8 subIndex, quint32 abortCode)
{
    ensureNode(nodeId);

    NodeSession &session = m_sessions[nodeId];
    session.lastSDOIndex = index;
    session.lastSDOSubIndex = subIndex;
    session.lastSDOAbortCode = abortCode;

    emit nodeUpdated(nodeId);
}

void NodeManager::onPDOReceived(quint32 cobId, const QByteArray &data)
{
    //check if pdo is registered to node
    if (!m_PDOCobIdToNode.contains(cobId))
    {
        return;
    }

    const quint8 nodeId = m_PDOCobIdToNode.value(cobId);
    ensureNode(nodeId);

    //store data
    m_sessions[nodeId].PDOValues[cobId] = data;

    emit nodeUpdated(nodeId);
}









