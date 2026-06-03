/**
 * @file ConnectionPool.cpp
 * @brief 连接池实现 — 多连接管理+自动重连+活动跟踪
 */

#include "connection/pool/ConnectionPool.h"
#include <QDateTime>

/** @brief 构造函数 - 初始化重连定时器 @param parent 父对象 */
ConnectionPool::ConnectionPool(QObject *parent) : QObject(parent), m_reconnectTimer(new QTimer(this)) {
    connect(m_reconnectTimer, &QTimer::timeout, this, &ConnectionPool::onReconnectTimer);
}

/** @brief 析构函数 - 断开所有连接 */
ConnectionPool::~ConnectionPool() { disconnectAll(); }

/** @brief 创建新连接条目 @param type 连接类型 @param addr 地址 @return 连接ID，池满返回空 */
QString ConnectionPool::createConnection(const QString &type, const QString &addr) {
    if (m_pool.size() >= m_maxConnections) { m_totalPoolFullEvents++; emit poolFull(); return {}; }
    m_totalCreated++;
    QString id = QString("conn_%1").arg(++m_counter);
    PoolEntry e; e.id = id; e.type = type; e.address = addr;
    e.created = QDateTime::currentMSecsSinceEpoch(); e.lastActivity = e.created;
    m_pool[id] = e;
    emit connectionCreated(id);
    return id;
}

/** @brief 移除连接 @param id 连接ID */
void ConnectionPool::removeConnection(const QString &id) {
    if (m_pool.remove(id)) { m_totalRemoved++; emit connectionRemoved(id); }
}

/** @brief 连接池中所有未连接条目 */
void ConnectionPool::connectAll() {
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (!it->connected) { it->connected = true; emit connectionStateChanged(it->id, true); }
    }
}

/** @brief 断开池中所有连接 */
void ConnectionPool::disconnectAll() {
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (it->connected) { it->connected = false; emit connectionStateChanged(it->id, false); }
    }
}

/** @brief 获取连接条目 @param id 连接ID @return 连接条目 */
ConnectionPool::PoolEntry ConnectionPool::connection(const QString &id) const { return m_pool.value(id); }

/** @brief 获取所有连接条目 @return 连接条目列表 */
QList<ConnectionPool::PoolEntry> ConnectionPool::allConnections() const { return m_pool.values(); }

/** @brief 按类型过滤连接 @param type 连接类型 @return 匹配的连接列表 */
QList<ConnectionPool::PoolEntry> ConnectionPool::connectionsByType(const QString &type) const {
    QList<PoolEntry> r; for (const auto &e : m_pool) if (e.type == type) r.append(e); return r;
}

/** @brief 获取已连接数量 @return 已连接数 */
int ConnectionPool::connectedCount() const { int c=0; for (const auto &e:m_pool) if (e.connected) c++; return c; }

/** @brief 获取总连接数量 @return 总数 */
int ConnectionPool::totalCount() const { return m_pool.size(); }

/** @brief 设置最大连接数 @param m 最大数量 */
void ConnectionPool::setMaxConnections(int m) { m_maxConnections = m; }

/** @brief 设置自动重连 @param enable 启用标志 @param interval 重连间隔(ms) */
void ConnectionPool::setAutoReconnect(bool enable, int interval) {
    m_autoReconnect = enable;
    if (enable) m_reconnectTimer->start(interval); else m_reconnectTimer->stop();
}

/** @brief 更新连接活动时间 @param id 连接ID */
void ConnectionPool::updateActivity(const QString &id) {
    auto it = m_pool.find(id);
    if (it != m_pool.end()) { m_totalActivityUpdates++; it->lastActivity = QDateTime::currentMSecsSinceEpoch(); }
}

/** @brief 自动重连定时器回调 — 重新连接所有断开的条目 */
void ConnectionPool::onReconnectTimer() {
    if (!m_autoReconnect) return;
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it) {
        if (!it->connected) { m_totalReconnectAttempts++; it->connected = true; emit connectionStateChanged(it->id, true); }
    }
}

/** @brief 重置所有统计计数器 */
void ConnectionPool::resetPoolStatistics() {
    m_totalCreated = 0;
    m_totalRemoved = 0;
    m_totalReconnectAttempts = 0;
    m_totalActivityUpdates = 0;
    m_totalPoolFullEvents = 0;
}
