#include "nmtmaster.h"

//internal vars - CiA 301 standard
namespace {
constexpr quint32 kNMTCommandCobId = 0x000;
constexpr quint32 kHeartbeatCobIdBase = 0x700;

constexpr quint8 kCmdStart = 0x01;
constexpr quint8 kCmdStop = 0x02;
constexpr quint8 kCmdEnterPreOperational = 0x80;
constexpr quint8 kCmdResetNode = 0x81;
constexpr quint8 kCmdResetCommunication = 0x82;

constexpr quint8 kStateBootUp = 0x00;
constexpr quint8 kStateStopped = 0x04;
constexpr quint8 kStateOperational = 0x05;
constexpr quint8 kStatePreOperational = 0x7F;
}

NMTMaster::NMTMaster(QObject *parent)
    : QObject{parent}
{}

void NMTMaster::startNode(quint8 nodeId) { sendCommand(kCmdStart, nodeId); }
void NMTMaster::stopNode(quint8 nodeId) { sendCommand(kCmdStop, nodeId); }
void NMTMaster::enterPreOperational(quint8 nodeId) { sendCommand(kCmdEnterPreOperational, nodeId); }
void NMTMaster::resetNode(quint8 nodeId) { sendCommand(kCmdResetNode, nodeId); }
void NMTMaster::resetCommunication(quint8 nodeId) { sendCommand(kCmdResetCommunication, nodeId); }

NodeState NMTMaster::state(quint8 nodeId) const
{
    //default return unknown state for new nodes
    return m_states.value(nodeId, NodeState::Unknown);
}

void NMTMaster::sendCommand(quint8 commandSpecifier, quint8 nodeId)
{
    //2 bytes, init to 0
    QByteArray payload(2, 0);

    payload[0] = static_cast<char>(commandSpecifier);
    payload[1] = static_cast<char>(nodeId);

    emit frameToSend(QCanBusFrame(kNMTCommandCobId, payload));
}

void NMTMaster::onFrameReceived(const QCanBusFrame &frame)
{
    const quint32 id = frame.frameId();

    //range check for right ID
    if (id < kHeartbeatCobIdBase || id > kHeartbeatCobIdBase + 0x7f)
    {
        return;
    }

    //pull node id
    const quint8 nodeId = static_cast<quint8>(id - kHeartbeatCobIdBase);

    if (frame.payload().isEmpty())
    {
        return;
    }

    const quint8 rawState = static_cast<quint8>(frame.payload().at(0));

    NodeState newState = NodeState::Unknown;

    switch (rawState) {
    case kStateBootUp:              newState = NodeState::BootUp; break;
    case kStateStopped:             newState = NodeState::Stopped; break;
    case kStatePreOperational:      newState = NodeState::PreOperational; break;
    case kStateOperational:         newState = NodeState::Operational; break;
    default:                        return;

    }

    if (m_states.value(nodeId, NodeState::Unknown) == newState)
    {
        //no changed state - don't spam signal
        return;
    }

    m_states[nodeId] = newState;
    emit nodeStateChanged(nodeId, newState);
}