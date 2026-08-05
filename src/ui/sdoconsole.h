#ifndef SDOCONSOLE_H
#define SDOCONSOLE_H

#include <QObject>
#include <QString>
#include "src/canopen/sdoclient.h"

//UI facing bridge for SDOClient
class SDOConsole : public QObject
{
    Q_OBJECT
    //q properties - read only
    Q_PROPERTY(QString lastResult READ lastResult NOTIFY lastResultChanged);
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged);

public:
    explicit SDOConsole(SDOClient *sdoClient, QObject *parent = nullptr);

    QString lastResult() const;
    bool busy() const;

    //values arrive from QML as ints - currently no bounds checking
    Q_INVOKABLE void readValue(int nodeId, int index, int subIndex);
    Q_INVOKABLE void writeValue(int nodeId, int index, int subIndex, const QString &hexData);

signals:
    void lastResultChanged();
    void busyChanged();

private slots:
    void onUploadFinished(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data);
    void onDownloadFinished(quint8 nodeId, quint16 index, quint8 subIndex);
    void onAborted(quint8 nodeId, quint16 index, quint8 subIndex, quint32 abortCode);

private:
    void setLastResult(const QString &text);
    void setBusy(bool value);

    SDOClient *m_sdoClient;
    QString m_lastResult;
    bool m_busy = false;
};

#endif // SDOCONSOLE_H
