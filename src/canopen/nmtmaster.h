#ifndef NMTMASTER_H
#define NMTMASTER_H

#include <QObject>
#include <QHash>
#include <QCanBusFrame>

enum class NodeState {
    Unknown,
    BootUp,
    Stopped,
    PreOperational,
    Operational
};

//Handles NMT states - sends and receives
class NMTMaster : public QObject
{
    Q_OBJECT

public:
    explicit NMTMaster(QObject *parent = nullptr);

    NodeState state(quint8 nodeId) const;

public slots:
    void startNode(quint8 nodeId);
    void stopNode(quint8 nodeId);
    void enterPreOperational(quint8 nodeId);
    void resetNode(quint8 nodeId);
    void resetCommunication(quint8 nodeId);

    void onFrameReceived(const QCanBusFrame &frame);

signals:
    void frameToSend(const QCanBusFrame &frame);
    void nodeStateChanged(quint8 nodeId, NodeState state);

private:
    void sendCommand(quint8 commandSpecifier, quint8 nodeId);

    QHash<quint8, NodeState> m_states;
};

#endif // NMTMASTER_H
