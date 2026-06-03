/**
 * @file ConnectionPool.cpp
 * @brief 连接池实现
 * @since score-132
 */
#include "connection/pool/ConnectionPool.h"
#include "connection/interface/IConnection.h"
#include <QDateTime>
#include <QMutexLocker>

ConnectionPool::ConnectionPool(QObject *parent) : QObject(parent) {
    m_cleanupTimer.setInterval(10000);
    connect(&m_cleanupTimer, &QTimer::timeout, this, &ConnectionPool::cleanupIdle);
}
ConnectionPool::~ConnectionPool() { shutdown(); }
ConnectionPool &ConnectionPool::instance() { static ConnectionPool inst; return inst; }
void ConnectionPool::initialize() { m_cleanupTimer.start(); }

std::shared_ptr<IConnection> ConnectionPool::acquire(const QString &connectionType, const QString &config) {
    QMutexLocker locker(&m_mutex);
    ++m_totalAcquires;
    // Try to reuse idle connection of same type
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (!it->inUse && it->connection) {
            it->inUse = true;
            it->lastUsedMs = QDateTime::currentMSecsSinceEpoch();
            ++m_totalReused;
            emit connectionAcquired(it->connectionId);
            return it->connection;
        }
    }
    // Create new
    ++m_totalCreated;
    auto conn = std::make_shared<IConnection>();
    PoolEntry entry;
    entry.connection = conn;
    entry.inUse = true;
    entry.lastUsedMs = QDateTime::currentMSecsSinceEpoch();
    entry.connectionId = generateId();
    m_pool[entry.connectionId] = entry;
    emit connectionAcquired(entry.connectionId);
    return conn;
}

void ConnectionPool::release(const QString &connectionId) {
    QMutexLocker locker(&m_mutex);
    auto it = m_pool.find(connectionId);
    if (it != m_pool.end()) {
        it->inUse = false;
        it->lastUsedMs = QDateTime::currentMSecsSinceEpoch();
        ++m_totalReleases;
        emit connectionReleased(connectionId);
    }
}

void ConnectionPool::releaseAll() {
    QMutexLocker locker(&m_mutex);
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (it->inUse) { it->inUse = false; it->lastUsedMs = QDateTime::currentMSecsSinceEpoch(); ++m_totalReleases; }
    }
}

void ConnectionPool::shutdown() {
    QMutexLocker locker(&m_mutex);
    m_cleanupTimer.stop();
    m_pool.clear();
    emit poolCleared();
}

int ConnectionPool::activeCount() const { QMutexLocker locker(&m_mutex); int c = 0; for (const auto &e : m_pool) if (e.inUse) ++c; return c; }
int ConnectionPool::idleCount() const { QMutexLocker locker(&m_mutex); int c = 0; for (const auto &e : m_pool) if (!e.inUse) ++c; return c; }
int ConnectionPool::totalCount() const { QMutexLocker locker(&m_mutex); return m_pool.size(); }
void ConnectionPool::setMaxIdlePerType(int maxIdle) { m_maxIdlePerType = qMax(1, maxIdle); }
int ConnectionPool::maxIdlePerType() const { return m_maxIdlePerType; }
void ConnectionPool::setIdleTimeoutMs(qint64 ms) { m_idleTimeoutMs = qMax(1000, ms); }
qint64 ConnectionPool::idleTimeoutMs() const { return m_idleTimeoutMs; }

quint64 ConnectionPool::totalAcquires() const { return m_totalAcquires; }
quint64 ConnectionPool::totalReleases() const { return m_totalReleases; }
quint64 ConnectionPool::totalCreated() const { return m_totalCreated; }
quint64 ConnectionPool::totalReused() const { return m_totalReused; }
quint64 ConnectionPool::totalExpired() const { return m_totalExpired; }
double ConnectionPool::reuseRate() const { return m_totalAcquires == 0 ? 0.0 : static_cast<double>(m_totalReused) / static_cast<double>(m_totalAcquires); }
void ConnectionPool::resetStatistics() { m_totalAcquires = 0; m_totalReleases = 0; m_totalCreated = 0; m_totalReused = 0; m_totalExpired = 0; }

void ConnectionPool::cleanupIdle() {
    QMutexLocker locker(&m_mutex);
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    auto it = m_pool.begin();
    while (it != m_pool.end()) {
        if (!it->inUse && (now - it->lastUsedMs) > m_idleTimeoutMs) {
            ++m_totalExpired;
            emit connectionExpired(it->connectionId);
            it = m_pool.erase(it);
        } else { ++it; }
    }
}

QString ConnectionPool::generateId() const { return QStringLiteral("cpool_%1").arg(++m_idCounter); }
