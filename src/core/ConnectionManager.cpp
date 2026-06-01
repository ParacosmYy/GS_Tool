/**
 * @file ConnectionManager.cpp
 * @brief 连接管理器实现 - 管理所有IConnection实例的生命周期
 *
 * 生命周期管理策略:
 *   - 创建时不传parent给工厂，由本管理器手动管理内存
 *   - removeConnection时先close()再deleteLater()，确保排队的槽执行完毕
 *   - 析构时关闭并删除所有活跃连接
 */

#include "core/ConnectionManager.h"
#include "core/ConnectionFactory.h"

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
