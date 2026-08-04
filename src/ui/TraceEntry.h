#ifndef TRACEENTRY_H
#define TRACEENTRY_H

#include <QtGlobal>
#include <QByteArray>

//Raw logged frame - add CANopen decoding later
struct TraceEntry {
    qint64 timestampMs = 0; //time since trace started
    quint32 cobId = 0;
    bool isTx = false;
    QByteArray payload;
};

#endif // TRACEENTRY_H
