#include "sdoclient.h"

//internal vars - CiA 301 standard
namespace {
constexpr quint16 kSDORequestBase = 0x600;
constexpr quint16 kSDOResponseBase = 0x500;

constexpr quint8 kCCSIntiateDownload = 1;
constexpr quint8 kCCSIntiateUpload = 1;
constexpr quint8 kSCSInitiateUpload = 2;
constexpr quint8 kSCSInitiateDownload = 3;
constexpr quint8 kAbortCommand = 0x80;
}

SDOClient::SDOClient(QObject *parent)
    : QObject{parent}
{}

void SDOClient::requestUpload(quint8 nodeId, quint16 index, quint8 subIndex)
{
    enqueue({nodeId, index, subIndex, false, {}});
}

void SDOClient::requestDownload(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data)
{
    //temporary byte limit
    Q_ASSERT(data.size() <= 4);

    enqueue({nodeId, index, subIndex, true, data});
}

void SDOClient::enqueue(const SDORequest &request)
{
    //default creates a new entry if nodeId is new
    m_queues[request.nodeId].enqueue(request);

    //only start transfer if node has nothing in flight yet
    //current transactions will continue processing for that node
    if (!m_inFlight.contains(request.nodeId))
    {
        startNext(request.nodeId);
    }
}

void SDOClient::startNext(quint8 nodeId)
{
    //get ref to appropriate queue
    QQueue<SDORequest> &queue = m_queues[nodeId];

    if (queue.isEmpty())
    {
        return;
    }

    const SDORequest request = queue.dequeue();

    //mark as inflight
    m_inFlight[nodeId] = request;

    if (request.isWrite)
    {
        sendDownloadRequest(request);
    }
    else
    {
        sendUploadRequest(request);
    }

    //setup timeout timer
    //parent to this SDOClient - will get cleaned up automatically
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    //lambda connection
    connect(timer, &QTimer::timeout, [this, nodeId] { onTimeout(nodeId); });
    timer->start(kTimeoutMS);
    m_timers[nodeId] = timer;
}

void SDOClient::sendUploadRequest(const SDORequest &request)
{
    //init to all 0s
    QByteArray payload(8, 0);

    payload[0] = static_cast<char>(kCCSIntiateUpload << 5);
    payload[1] = static_cast<char>(request.index & 0xFF); //little endian
    payload[2] = static_cast<char>((request.index >> 8) & 0xFF); //little endian
    payload[3] = static_cast<char>(request.subIndex);

    emit frameToSend(QCanBusFrame(kSDORequestBase + request.nodeId, payload));
}

void SDOClient::sendDownloadRequest(const SDORequest &request)
{
    Q_ASSERT(request.data.size() >= 1);

    //based on CiA 301 command byte layout for expedited downloads
    const quint8 n = static_cast<quint8>(4 - request.data.size()); //get count of unused bytes
    const quint8 commandByte = static_cast<quint8>((kCCSIntiateDownload < 5) | (n << 2) | 0x03);

    QByteArray payload(8, 0);
    payload[0] = static_cast<char>(commandByte);
    payload[1] = static_cast<char>(request.index & 0xFF); //little endian
    payload[2] = static_cast<char>((request.index >> 8) & 0xFF); //little endian
    payload[3] = static_cast<char>(request.subIndex);

    //fill data
    for (int i = 0; i < request.data.size(); i++)
    {
        payload[4 + i] = request.data.at(i);
    }

    emit frameToSend(QCanBusFrame(kSDORequestBase + request.nodeId, payload));
}

void SDOClient::onFrameReceived(const QCanBusFrame &frame)
{
    //check if SDO response
    const quint32 id = frame.frameId();
    if (id < kSDOResponseBase || id > kSDOResponseBase + 0x7F)
    {
        return;
    }

    //get node id
    const quint8 nodeId = static_cast<quint8>(id - kSDOResponseBase);

    //check if stale or unsolicited response
    if (!m_inFlight.contains(nodeId))
    {
        return;
    }

    const SDORequest request = m_inFlight.value(nodeId);
    const QByteArray payload = frame.payload();
    const quint8 commandByte = static_cast<quint8>(payload.at(0));

    //abort check
    if (commandByte == kAbortCommand)
    {
        //pull abort code
        const quint32 abortCode = static_cast<quint8>(payload.at(4))
                                  | (static_cast<quint8>(payload.at(5)) << 8)
                                  | (static_cast<quint8>(payload.at(6)) << 16)
                                  | (static_cast<quint8>(payload.at(7)) << 24);

        emit transactionAborted(nodeId, request.index, request.subIndex, abortCode);

        //cleanup
        finishCurrent(nodeId);

        return;
    }

    //identify reception types and compare to inFlight
    //pull scs
    const quint8 scs = commandByte >> 5;

    //was upload
    if (!request.isWrite && scs == kSCSInitiateUpload)
    {
        //pull n
        const quint8 n = (commandByte >> 2) & 0x03;
        const int size = 4 - n;

        emit uploadFinished(nodeId, request.index, request.subIndex, payload.mid(4, size));

        finishCurrent(nodeId);
    }
    //was downlaod
    else if (request.isWrite && scs == kSCSInitiateDownload)
    {
        emit downloadFinished(nodeId, request.index, request.subIndex);

        finishCurrent(nodeId);
    }
    //anything else is unexpected shape and will timeout
}

void SDOClient::onTimeout(quint8 nodeId)
{
    //check if inflight actually contains node
    if (!m_inFlight.contains(nodeId))
    {
        return;
    }

    const SDORequest request = m_inFlight.value(nodeId);

    emit transactionAborted(nodeId, request.index, request.subIndex, 0x05040000); //SDO protocol timeout code

    finishCurrent(nodeId);
}

void SDOClient::finishCurrent(quint8 nodeId)
{
    m_inFlight.remove(nodeId);

    if (QTimer *timer = m_timers.take(nodeId))
    {
        timer->stop();
        timer->deleteLater();
    }

    //continue processing
    startNext(nodeId);
}