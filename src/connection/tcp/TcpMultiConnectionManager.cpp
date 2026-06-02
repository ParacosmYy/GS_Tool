/**
 * @file TcpMultiConnectionManager.cpp
 * @brief TCP多连接管理器实现 - 骨架
 */

#include "connection/tcp/TcpMultiConnectionManager.h"
#include "connection/interface/IConnection.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TcpMultiConnectionManager::TcpMultiConnectionManager(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数 - 关闭所有连接
 */
TcpMultiConnectionManager::~TcpMultiConnectionManager()
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value()) {
            it.value()->close();
            it.value()->deleteLater();
        }
    }
    m_connections.clear();
}

/**
 * @brief 添加新的TCP连接
 * @param host 目标主机地址
 * @param port 目标端口号
 * @return 新连接ID，-1表示失败
 */
int TcpMultiConnectionManager::addConnection(const QString& host, int port)
{
    Q_UNUSED(host)
    Q_UNUSED(port)
    // TODO: 创建IConnection实例，配置并打开连接
    int id = m_nextId++;
    // IConnection* conn = ConnectionFactory::create(ConnectionType::TcpClient, this);
    // conn->configure({{"host", host}, {"port", port}});
    // m_connections[id] = conn;
    emit connectionAdded(id);
    return id;
}

/**
 * @brief 移除指定连接
 * @param id 连接ID
 */
void TcpMultiConnectionManager::removeConnection(int id)
{
    if (m_connections.contains(id)) {
        IConnection* conn = m_connections.take(id);
        if (conn) {
            conn->close();
            conn->deleteLater();
        }
        emit connectionRemoved(id);
    }
}

/**
 * @brief 获取当前连接数量
 * @return 活跃连接数
 */
int TcpMultiConnectionManager::connectionCount() const
{
    return m_connections.size();
}

/**
 * @brief 向所有连接发送数据
 * @param data 待发送的字节数据
 * @return 成功发送的连接数
 */
int TcpMultiConnectionManager::sendToAll(const QByteArray& data)
{
    int count = 0;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value() && it.value()->write(data) > 0) {
            count++;
        }
    }
    return count;
}

/**
 * @brief 连接数据到达的统一处理
 * @param data 收到的数据
 */
void TcpMultiConnectionManager::onDataReceived(const QByteArray& data)
{
    // TODO: 识别来源连接ID并发射dataReceived信号
    Q_UNUSED(data)
}
