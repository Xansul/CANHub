#include "sdoconsole.h"

SDOConsole::SDOConsole(SDOClient *sdoClient, QObject *parent)
    : QObject{parent}, m_sdoClient{sdoClient}
{
    //wire up connections
    connect(sdoClient, &SDOClient::uploadFinished, this, &SDOConsole::onUploadFinished);
    connect(sdoClient, &SDOClient::downloadFinished, this, &SDOConsole::onDownloadFinished);
    connect(sdoClient, &SDOClient::transactionAborted, this, &SDOConsole::onAborted);
}

QString SDOConsole::lastResult() const
{
    return m_lastResult;
}

bool SDOConsole::busy() const
{
    return m_busy;
}

void SDOConsole::readValue(int nodeId, int index, int subIndex)
{
    setBusy(true);

    m_sdoClient->requestUpload(static_cast<quint8>(nodeId), static_cast<quint16>(index), static_cast<quint8>(subIndex));
}

void SDOConsole::writeValue(int nodeId, int index, int subIndex, const QString &hexData)
{
    //convert to byte array
    QByteArray cleaned = hexData.toUtf8();
    //remove spaces
    cleaned.replace(" ", "");
    const QByteArray data = QByteArray::fromHex(cleaned);

    //safety check
    if (data.isEmpty() || data.size() > 4)
    {
        //use same result logic for error
        setLastResult(QStringLiteral("Invalid data - enter 1 to 4 hex bytes"));
        return;
    }

    setBusy(true);

    m_sdoClient->requestDownload(static_cast<quint8>(nodeId), static_cast<quint16>(index), static_cast<quint8>(subIndex), data);
}

void SDOConsole::onUploadFinished(quint8 nodeId, quint16 index, quint8 subIndex, const QByteArray &data)
{
    setBusy(false);

    setLastResult(QStringLiteral("Node %1 0x%2:%3 = %4").arg(nodeId).arg(index, 4, 16, QLatin1Char('0')).arg(subIndex).arg(QString::fromLatin1(data.toHex(' ').toUpper())));
}

void SDOConsole::onDownloadFinished(quint8 nodeId, quint16 index, quint8 subIndex)
{
    setBusy(false);

    setLastResult(QStringLiteral("Node %1 0x%2:%3 write OK").arg(nodeId).arg(index, 4, 16, QLatin1Char('0')).arg(subIndex));
}

void SDOConsole::onAborted(quint8 nodeId, quint16 index, quint8 subIndex, quint32 abortCode)
{
    setBusy(false);

    setLastResult(QStringLiteral("Node %1 0x%2:%3 ABORTED (0x%4)").arg(nodeId).arg(index, 4, 16, QLatin1Char('0')).arg(subIndex).arg(abortCode, 8, 16, QLatin1Char('0')));
}

void SDOConsole::setLastResult(const QString &text)
{
    //check if same
    if (m_lastResult == text)
    {
        return;
    }

    m_lastResult = text;
    emit lastResultChanged();
}

void SDOConsole::setBusy(bool value)
{
    //check if same
    if (m_busy == value)
    {
        return;
    }

    m_busy = value;
    emit busyChanged();
}