#ifndef PDOENGINE_H
#define PDOENGINE_H

#include <QObject>
#include <QSet>
#include <QHash>
#include <QCanBusFrame>

//Raw PDO handling engine - TPDOs and RPDOs - currently no parsing of meaning - operates on raw bytes
class PDOEngine : public QObject
{
    Q_OBJECT

public:
    explicit PDOEngine(QObject *parent = nullptr);

    //start tracking PDO
    void subscribe(quint32 cobId);
    //stop tracking PDO
    void unsubscribe(quint32 cobId);

    //gives last received payload
    QByteArray lastValue(quint32 cobId) const;

    void sendPDO(quint32 cobId, const QByteArray &data);

    //request via RTR frame - not all nodes support this
    void requestPDO(quint32 cobId);

public slots:
    void onFrameReceived(const QCanBusFrame &frame);
    \
signals:
    void frameToSend(const QCanBusFrame &frame);
    void PDOReceived(quint32 cobId, const QByteArray &data);

private:
    QSet<quint32> m_subscriptions;
    QHash<quint32, QByteArray> m_lastValues;
};

#endif // PDOENGINE_H
