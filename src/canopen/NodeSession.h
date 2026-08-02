#ifndef NODESESSION_H
#define NODESESSION_H

#include <QtGlobal>
#include <QByteArray>
#include <QHash>
#include "nmtmaster.h"

//Snapshot of node - this is what UI binds to
struct NodeSession {
    quint8 nodeId = 0;
    NodeState NMTState = NodeState::Unknown;
    bool alive = false;

    //most recent successful SDO upload
    quint16 lastSDOIndex = 0;
    quint8 lastSDOSubIndex = 0;
    QByteArray lastSDOValue;

    //0 means no abort seen
    quint32 lastSDOAbortCode = 0;

    //last known payload per subscribed PDO
    QHash<quint32, QByteArray> PDOValues;
};

#endif // NODESESSION_H
