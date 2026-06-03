/**
 * @file ConnectionManager.cpp
 * @brief 连接管理器实现 - 管理所有IConnection实例的生命周期
 *
 * 生命周期管理策略:
 *   - 创建时不传parent给工厂，由本管理器手动管理内存
 *   - removeConnection时先close()再deleteLater()，确保排队的槽执行完毕
 *   - 析构时关闭并删除所有活跃连接
 */

#include "core/connect/ConnectionManager.h"
#include "core/connect/ConnectionFactory.h"

/** @brief 构造连接管理器 */
ConnectionManager::ConnectionManager(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构，关闭并删除所有活跃连接
 *
 * 遍历所有连接，依次close() + delete，确保资源正确释放。
 * 连接对象没有设parent，需要手动delete避免内存泄漏。
 */
ConnectionManager::~ConnectionManager()
{
    for (auto* conn : m_connections) {
        conn->close();
        delete conn;
    }
    m_connections.clear();
}

/**
 * @brief 创建指定类型的连接（委托给ConnectionFactory）
 *
 * 不传parent给工厂，由ConnectionManager手动管理生命周期，避免双重删除。
 * 创建成功后自动加入内部连接列表。
 *
 * @param type 连接类型枚举
 * @return 新创建的连接实例，失败返回nullptr
 */
IConnection* ConnectionManager::createConnection(ConnectionType type)
{
    auto* conn = ConnectionFactory::create(type, nullptr);
    if (conn) {
        m_connections.append(conn);
        m_activeConnection = conn;

        // 统计：累计创建连接计数
        ++m_totalConnectionsCreated;
    }
    return conn;
}

/**
 * @brief 串口便捷方法（内部委托给通用createConnection）
 * @return 新创建的串口连接实例
 */
IConnection* ConnectionManager::createSerialConnection()
{
    return createConnection(ConnectionType::Serial);
}

/**
 * @brief 删除一个连接（自动close + deleteLater）
 *
 * 使用deleteLater而非直接delete，确保所有已排队的信号槽执行完毕后再销毁对象。
 * 如果连接不在管理列表中，则不做任何操作。
 *
 * @param conn 要删除的连接指针
 */
void ConnectionManager::removeConnection(IConnection* conn)
{
    if (m_connections.removeOne(conn)) {
        // 统计：累计销毁连接计数
        ++m_totalConnectionsDestroyed;

        // 如果删除的是活跃连接，自动切换到列表中最后一个
        if (m_activeConnection == conn) {
            m_activeConnection = m_connections.isEmpty()
                ? nullptr
                : m_connections.last();
        }
        conn->close();
        conn->deleteLater();
    }
}

/** @brief 返回所有活跃连接列表的副本 */
QList<IConnection*> ConnectionManager::connections() const
{
    return m_connections;
}

/**
 * @brief 按索引获取连接
 * @param index 连接索引
 * @return 连接指针，越界返回nullptr
 */
IConnection* ConnectionManager::connection(int index) const
{
    if (index >= 0 && index < m_connections.size()) {
        return m_connections[index];
    }
    return nullptr;
}

/** @brief 返回当前活跃连接数量 */
int ConnectionManager::count() const
{
    return m_connections.size();
}

/**
 * @brief 切换活跃连接到指定实例
 *
 * 目标连接必须已存在于活跃列表中，否则不做任何操作。
 *
 * @param conn 目标连接指针
 */
void ConnectionManager::switchActiveConnection(IConnection* conn)
{
    if (!conn || !m_connections.contains(conn)) {
        return;
    }
    if (m_activeConnection == conn) {
        return;
    }

    m_activeConnection = conn;

    // 统计：累计切换计数
    ++m_totalSwitches;
}

/** @brief 获取当前活跃连接 */
IConnection* ConnectionManager::activeConnection() const
{
    return m_activeConnection;
}

// ==================== 统计接口 ====================

/** @brief 获取累计创建连接总数 */
quint64 ConnectionManager::totalConnectionsCreated() const
{
    return m_totalConnectionsCreated;
}

/** @brief 获取累计销毁连接总数 */
quint64 ConnectionManager::totalConnectionsDestroyed() const
{
    return m_totalConnectionsDestroyed;
}

/** @brief 获取累计切换连接次数 */
quint64 ConnectionManager::totalSwitches() const
{
    return m_totalSwitches;
}

/** @brief 重置所有统计计数器为零 */
void ConnectionManager::resetStats()
{
    m_totalConnectionsCreated = 0;
    m_totalConnectionsDestroyed = 0;
    m_totalSwitches = 0;
}
