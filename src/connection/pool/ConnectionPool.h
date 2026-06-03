/**
 * @file ConnectionPool.h
 * @brief 连接池 - 管理可复用的IConnection连接实例
 * @since score-132
 */
#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <memory>

class IConnection;

struct PoolEntry {
    std::shared_ptr<IConnection> connection;
    bool inUse = false;
    qint64 lastUsedMs = 0;
    QString connectionId;
};

class ConnectionPool : public QObject {
    Q_OBJECT
public:
    static ConnectionPool &instance();
    void initialize();

    std::shared_ptr<IConnection> acquire(const QString &connectionType, const QString &config);
    void release(const QString &connectionId);
    void releaseAll();
    void shutdown();

    int activeCount() const;
    int idleCount() const;
    int totalCount() const;
    void setMaxIdlePerType(int maxIdle);
    int maxIdlePerType() const;
    void setIdleTimeoutMs(qint64 ms);
    qint64 idleTimeoutMs() const;

    quint64 totalAcquires() const;
    quint64 totalReleases() const;
    quint64 totalCreated() const;
    quint64 totalReused() const;
    quint64 totalExpired() const;
    double reuseRate() const;
    void resetStatistics();

signals:
    void connectionAcquired(const QString &connectionId);
    void connectionReleased(const QString &connectionId);
    void connectionExpired(const QString &connectionId);
    void poolCleared();

private:
    explicit ConnectionPool(QObject *parent = nullptr);
    ~ConnectionPool() override;
    ConnectionPool(const ConnectionPool &) = delete;
    ConnectionPool &operator=(const ConnectionPool &) = delete;

    void cleanupIdle();
    QString generateId() const;

    mutable QMutex m_mutex;
    QMap<QString, PoolEntry> m_pool;
    QTimer m_cleanupTimer;
    int m_maxIdlePerType = 5;
    qint64 m_idleTimeoutMs = 30000;

    mutable quint64 m_totalAcquires = 0;
    mutable quint64 m_totalReleases = 0;
    mutable quint64 m_totalCreated = 0;
    mutable quint64 m_totalReused = 0;
    mutable quint64 m_totalExpired = 0;
    mutable quint64 m_idCounter = 0;
};
#endif // CONNECTIONPOOL_H
