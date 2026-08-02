#ifndef SDOCLIENT_H
#define SDOCLIENT_H

#include <QObject>
#include <QCanBusFrame>
#include <QQueue>
#include <QHash>
#include <QTimer>

//NOTE only handling payload <= 4 bytes for now
struct SDORequest
{
    quint8 nodeId;
    quint16 index;
    quint8 subIndex;
    bool isWrite;
    QByteArray data;
};

//Handles SDO transfers - queues and handles messages based on Node ID
class SDOClient : public QObject
{
    Q_OBJECT

public:
    explicit SDOClient(QObject *parent = nullptr);

    //actual functions the UI calls
    //server->client
    void requestUpload(quint8 nodeId, quint16 index, quint8 subIndex);
    //client->server
    void requestDownload(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data);

public slots:
    void onFrameReceived(const QCanBusFrame &frame);

signals:
    void frameToSend(const QCanBusFrame &frame);

    void uploadFinished(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data);
    void downloadFinished(quint8 nodeId, quint16 index, quint8 subIndex);
    void transactionAborted(quint8 nodeId, quint16 index, quint8 subIndex, quint32 abortCode);

private:
    void enqueue(const SDORequest &request);
    void startNext(quint8 nodeId);
    void sendUploadRequest(const SDORequest &request);
    void sendDownloadRequest(const SDORequest &request);
    void finishCurrent(quint8 nodeId);
    void onTimeout(quint8 nodeId);

    QHash<quint8, QQueue<SDORequest>> m_queues;
    QHash<quint8, SDORequest> m_inFlight;
    QHash<quint8, QTimer*> m_timers;

    //timeout - possibly add adjustment later
    static constexpr int kTimeoutMS = 1000;
};

#endif // SDOCLIENT_H
