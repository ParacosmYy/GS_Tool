#include "ConnectionManager.h"
#include "connection/SerialConnection.h"

ConnectionManager::ConnectionManager(QObject* parent)
    : QObject(parent)
{
}

ConnectionManager::~ConnectionManager()
{
    // 关闭并删除所有连接
    for (auto* conn : m_connections) {
        conn->close();
        delete conn;
    }
    m_connections.clear();
}

IConnection* ConnectionManager::createSerialConnection()
{
    auto* conn = new SerialConnection(this);
    m_connections.append(conn);
    emit connectionAdded(conn);
    return conn;
}

void ConnectionManager::removeConnection(IConnection* conn)
{
    if (m_connections.removeOne(conn)) {
        conn->close();
        conn->deleteLater();
        emit connectionRemoved(conn);
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
