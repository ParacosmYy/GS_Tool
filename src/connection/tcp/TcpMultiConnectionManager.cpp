/**
 * @file TcpMultiConnectionManager.cpp
 * @brief TCP多连接管理器实现
 */

#include "connection/tcp/TcpMultiConnectionManager.h"
#include <QHostAddress>

/** @brief 构造TCP多连接管理器 @param parent 父QObject指针 */
TcpMultiConnectionManager::TcpMultiConnectionManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构管理器，断开并释放所有TCP连接 */
TcpMultiConnectionManager::~TcpMultiConnectionManager()
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
    m_connections.clear();
    m_hosts.clear();
    m_ports.clear();
}

/** @brief 添加一条新的TCP连接 @param host 目标主机地址 @param port 目标端口号 @return 新连接的ID */
int TcpMultiConnectionManager::addConnection(const QString& host, int port)
{
    auto* socket = new QTcpSocket(this);
    int id = m_nextId++;

    m_connections[id] = socket;
    m_hosts[id] = host;
    m_ports[id] = port;
    ++m_totalConnections;

    connect(socket, &QTcpSocket::readyRead,
            this, &TcpMultiConnectionManager::onReadyRead);
    connect(socket, &QTcpSocket::disconnected,
            this, &TcpMultiConnectionManager::onDisconnected);
    connect(socket, &QAbstractSocket::errorOccurred,
            this, &TcpMultiConnectionManager::onError);

    socket->connectToHost(QHostAddress(host), static_cast<quint16>(port));
    emit connectionAdded(id, host, port);
    return id;
}

/** @brief 移除并关闭指定ID的TCP连接 @param id 要移除的连接ID */
void TcpMultiConnectionManager::removeConnection(int id)
{
    if (!m_connections.contains(id)) return;
    ++m_totalDisconnections;

    QTcpSocket* socket = m_connections.take(id);
    m_hosts.remove(id);
    m_ports.remove(id);

    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    emit connectionRemoved(id);
}

/** @brief 获取当前活跃的TCP连接数量 @return 连接数 */
int TcpMultiConnectionManager::connectionCount() const
{
    return m_connections.size();
}

/** @brief 向所有已连接的socket广播数据 @param data 要发送的数据 @return 成功发送的socket数量 */
int TcpMultiConnectionManager::sendToAll(const QByteArray& data)
{
    int count = 0;
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = socket->write(data);
            if (written > 0) {
                m_totalBytesSent += static_cast<quint64>(written);
                socket->flush();
                count++;
            }
        }
    }
    return count;
}

/** @brief 获取指定连接的主机地址 @param id 连接ID @return 主机地址字符串 */
QString TcpMultiConnectionManager::connectionHost(int id) const
{
    return m_hosts.value(id, QString());
}

/** @brief 获取指定连接的端口号 @param id 连接ID @return 端口号，不存在返回-1 */
int TcpMultiConnectionManager::connectionPort(int id) const
{
    return m_ports.value(id, -1);
}

/** @brief 获取累计连接总数 @return 历史连接总数 */
quint64 TcpMultiConnectionManager::totalConnections() const { return m_totalConnections; }
/** @brief 获取累计断开总数 @return 历史断开总数 */
quint64 TcpMultiConnectionManager::totalDisconnections() const { return m_totalDisconnections; }
/** @brief 获取累计发送字节数 @return 发送字节总量 */
quint64 TcpMultiConnectionManager::totalBytesSent() const { return m_totalBytesSent; }
/** @brief 获取累计接收字节数 @return 接收字节总量 */
quint64 TcpMultiConnectionManager::totalBytesReceived() const { return m_totalBytesReceived; }
/** @brief 获取累计错误次数 @return 错误总数 */
quint64 TcpMultiConnectionManager::errorCount() const { return m_errorCount; }

/** @brief 重置所有连接统计计数器 */
void TcpMultiConnectionManager::resetConnectionStatistics()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}

/** @brief socket数据到达回调，累计接收字节数并转发数据信号 */
void TcpMultiConnectionManager::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id < 0) return;

    QByteArray data = socket->readAll();
    if (!data.isEmpty()) {
        m_totalBytesReceived += static_cast<quint64>(data.size());
        emit dataReceived(id, data);
    }
}

/** @brief socket断开回调，自动移除连接 */
void TcpMultiConnectionManager::onDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        removeConnection(id);
    }
}

/** @brief socket错误回调，累计错误计数并转发错误信号 @param error socket错误类型 */
void TcpMultiConnectionManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int id = idForSocket(socket);
    if (id >= 0) {
        ++m_errorCount;
        emit connectionError(id, socket->errorString());
    }
}

/** @brief 反向查找socket对象对应的连接ID @param socket QTcpSocket指针 @return 连接ID，未找到返回-1 */
int TcpMultiConnectionManager::idForSocket(QTcpSocket* socket) const
{
    for (auto it = m_connections.constBegin(); it != m_connections.constEnd(); ++it) {
        if (it.value() == socket) {
            return it.key();
        }
    }
    return -1;
}
