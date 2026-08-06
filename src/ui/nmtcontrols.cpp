#include "nmtcontrols.h"

NMTControls::NMTControls(NMTMaster *nmtMaster, QObject *parent)
    : QObject{parent}, m_nmtMaster{nmtMaster}
{}

void NMTControls::start(int nodeId)
{
    m_nmtMaster->startNode(static_cast<quint8>(nodeId));
}

void NMTControls::stop(int nodeId)
{
    m_nmtMaster->stopNode(static_cast<quint8>(nodeId));
}

void NMTControls::enterPreOperational(int nodeId)
{
    m_nmtMaster->enterPreOperational(static_cast<quint8>(nodeId));
}

void NMTControls::resetNode(int nodeId)
{
    m_nmtMaster->resetNode(static_cast<quint8>(nodeId));
}

void NMTControls::resetCommunication(int nodeId)
{
    m_nmtMaster->resetCommunication(static_cast<quint8>(nodeId));
}