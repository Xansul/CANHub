#ifndef PDOCONSOLEMODEL_H
#define PDOCONSOLEMODEL_H

#include <QAbstractListModel>
#include "src/canopen/pdoengine.h"
#include "src/canopen/nodemanager.h"

//UI facing bridge for PDO engine + list model for subscribed PODs - list is authoritative only for subscriptions made through this class
class PDOConsoleModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        COBIdRole = Qt::UserRole + 1,
        NodeIdRole,
        DataHexRole
    };

    explicit PDOConsoleModel(PDOEngine *pdoEngine, NodeManager *nodeManager, QObject *parent = nullptr);

    //overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    //q invokables
    Q_INVOKABLE void addSubscription(int cobId, int nodeId);
    Q_INVOKABLE void removeSubscription(int cobId);
    Q_INVOKABLE void sendPDO(int cobId, const QString &hexData);
    Q_INVOKABLE void pollPDO(int cobId);

private slots:
    void onPDOReceived(quint32 cobId, const QByteArray &data);

private:
    struct PDO {
        quint32 cobId = 0;
        quint8 nodeId = 0;
        QByteArray lastValue;
    };

    int rowForCobId(quint32 cobId) const;

    PDOEngine *m_pdoEngine;
    NodeManager *m_nodeManager;
    QList<PDO> m_PDOs;
};

#endif // PDOCONSOLEMODEL_H
