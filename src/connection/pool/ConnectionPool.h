#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QTimer>

class ConnectionPool : public QObject {
    Q_OBJECT
public:
    struct PoolEntry {
        QString id;
        QString type;
        QString address;
        bool connected = false;
        qint64 created = 0;
        qint64 lastActivity = 0;
    };

    explicit ConnectionPool(QObject *parent = nullptr);
    ~ConnectionPool() override;
    QString createConnection(const QString &type, const QString &address);
    void removeConnection(const QString &id);
    void connectAll();
    void disconnectAll();
    PoolEntry connection(const QString &id) const;
    QList<PoolEntry> allConnections() const;
    QList<PoolEntry> connectionsByType(const QString &type) const;
    int connectedCount() const;
    int totalCount() const;
    void setMaxConnections(int max);
    void setAutoReconnect(bool enable, int intervalMs = 5000);
    void updateActivity(const QString &id);
signals:
    void connectionCreated(const QString &id);
    void connectionRemoved(const QString &id);
    void connectionStateChanged(const QString &id, bool connected);
    void poolFull();
private:
    void onReconnectTimer();
    QMap<QString, PoolEntry> m_pool;
    int m_maxConnections = 16;
    bool m_autoReconnect = false;
    QTimer *m_reconnectTimer = nullptr;
    int m_counter = 0;
};
