#ifndef NMTCONTROLS_H
#define NMTCONTROLS_H

#include <QObject>
#include "src/canopen/nmtmaster.h"

//UI facing bridge for NMTMaster - doesn't do much but exists for future validation/logic
class NMTControls : public QObject
{
    Q_OBJECT

public:
    explicit NMTControls(NMTMaster *nmtMaster, QObject *parent = nullptr);

    //q invokables - input of nodeId 0 means broadcast to all nodes
    Q_INVOKABLE void start(int nodeId);
    Q_INVOKABLE void stop(int nodeId);
    Q_INVOKABLE void enterPreOperational(int nodeId);
    Q_INVOKABLE void resetNode(int nodeId);
    Q_INVOKABLE void resetCommunication(int nodeId);

private:
    NMTMaster *m_nmtMaster;
};

#endif // NMTCONTROLS_H
