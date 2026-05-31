#include "ConnectionManager.h"
#include "ConnectionFactory.h"

ConnectionManager::ConnectionManager(QObject* parent)
    : QObject(parent)
{
}

ConnectionManager::~ConnectionManager()
{
    // 关闭并删除所有连接
    // 注意: 连接对象没有设parent，需要手动delete
    for (auto* conn : m_connections) {
        conn->close();
        delete conn;
    }
    m_connections.clear();
}

// 创建指定类型的连接（委托给ConnectionFactory）
IConnection* ConnectionManager::createConnection(ConnectionType type)
{
    // 不传parent，由ConnectionManager手动管理生命周期，避免双重删除
    auto* conn = ConnectionFactory::create(type, nullptr);
    if (conn) {
        m_connections.append(conn);
    }
    return conn;
}

// 保留旧接口的兼容性，内部委托给通用方法
IConnection* ConnectionManager::createSerialConnection()
{
    return createConnection(ConnectionType::Serial);
}

void ConnectionManager::removeConnection(IConnection* conn)
{
    if (m_connections.removeOne(conn)) {
        conn->close();
        conn->deleteLater();           // deleteLater确保所有已排队的槽执行完毕后再销毁
    }
}

QList<IConnection*> ConnectionManager::connections() const
{
    return m_connections;
}

IConnection* ConnectionManager::connection(int index) const
{
    if (index >= 0 && index < m_connections.size()) {
        return m_connections[index];
    }
    return nullptr;
}

int ConnectionManager::count() const
{
    return m_connections.size();
}
