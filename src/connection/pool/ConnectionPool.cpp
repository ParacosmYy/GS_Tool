#include "connection/pool/ConnectionPool.h"
#include <QDateTime>

ConnectionPool::ConnectionPool(QObject *parent) : QObject(parent), m_reconnectTimer(new QTimer(this)) {
    connect(m_reconnectTimer, &QTimer::timeout, this, &ConnectionPool::onReconnectTimer);
}
ConnectionPool::~ConnectionPool() { disconnectAll(); }

QString ConnectionPool::createConnection(const QString &type, const QString &addr) {
    if (m_pool.size() >= m_maxConnections) { emit poolFull(); return {}; }
    QString id = QString("conn_%1").arg(++m_counter);
    PoolEntry e; e.id = id; e.type = type; e.address = addr;
    e.created = QDateTime::currentMSecsSinceEpoch(); e.lastActivity = e.created;
    m_pool[id] = e;
    emit connectionCreated(id);
    return id;
}

void ConnectionPool::removeConnection(const QString &id) {
    if (m_pool.remove(id)) emit connectionRemoved(id);
}

void ConnectionPool::connectAll() {
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (!it->connected) { it->connected = true; emit connectionStateChanged(it->id, true); }
    }
}

void ConnectionPool::disconnectAll() {
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (it->connected) { it->connected = false; emit connectionStateChanged(it->id, false); }
    }
}

ConnectionPool::PoolEntry ConnectionPool::connection(const QString &id) const { return m_pool.value(id); }
QList<ConnectionPool::PoolEntry> ConnectionPool::allConnections() const { return m_pool.values(); }

QList<ConnectionPool::PoolEntry> ConnectionPool::connectionsByType(const QString &type) const {
    QList<PoolEntry> r; for (const auto &e : m_pool) if (e.type == type) r.append(e); return r;
}

int ConnectionPool::connectedCount() const { int c=0; for (const auto &e:m_pool) if (e.connected) c++; return c; }
int ConnectionPool::totalCount() const { return m_pool.size(); }
void ConnectionPool::setMaxConnections(int m) { m_maxConnections = m; }

void ConnectionPool::setAutoReconnect(bool enable, int interval) {
    m_autoReconnect = enable;
    if (enable) m_reconnectTimer->start(interval); else m_reconnectTimer->stop();
}

void ConnectionPool::updateActivity(const QString &id) {
    auto it = m_pool.find(id);
    if (it != m_pool.end()) it->lastActivity = QDateTime::currentMSecsSinceEpoch();
}

void ConnectionPool::onReconnectTimer() {
    if (!m_autoReconnect) return;
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (!it->connected) { it->connected = true; emit connectionStateChanged(it->id, true); }
    }
}
