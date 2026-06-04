/**
 * @file TcpServerConnectionHandlers.cpp
 * @brief TCP服务器连接 — 客户端事件处理器实现
 *
 * 从 TcpServerConnection.cpp 拆分而来，包含:
 *   - onNewConnection: 新客户端连接回调(注册socket/信号/峰值统计)
 *   - onClientDisconnected: 客户端断开回调(移除映射/释放socket)
 *   - onClientReadyRead: 客户端数据到达回调(发射dataReceived信号)
 *   - updateState: 连接状态更新(发射stateChanged信号)
 *   - clientInfo: socket到"地址:端口"标识转换
 *
 * 连接生命周期和配置接口见 TcpServerConnection.cpp。
 * 客户端管理和统计查询见 TcpServerConnectionClients.cpp。
 */

#include "connection/tcp/TcpServerConnection.h"

/** @brief 新客户端连接回调，注册socket并连接断开/数据到达/错误信号 */
void TcpServerConnection::onNewConnection()
{
    if (!m_server) return;

    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        if (!client) continue;

        // 检查是否达到最大客户端数限制
        if (m_maxClients > 0 && m_clients.size() >= m_maxClients) {
            ++m_totalRejectedConnections;  // 因达到最大连接数拒绝
            client->abort();
            client->deleteLater();
            continue;
        }

        qintptr sd = client->socketDescriptor();
        m_clients[sd] = client;

        connect(client, &QTcpSocket::disconnected,
                this, &TcpServerConnection::onClientDisconnected);
        connect(client, &QTcpSocket::readyRead,
                this, &TcpServerConnection::onClientReadyRead);
        connect(client, &QAbstractSocket::errorOccurred,
                this, [this](QAbstractSocket::SocketError err) {
                    Q_UNUSED(err)
                    auto* socket = qobject_cast<QTcpSocket*>(sender());
                    if (socket) {
                        ++m_totalAcceptErrors;  // 客户端socket错误计数
                        ++m_totalErrors;        // 累计客户端错误总数
                        emit errorOccurred(tr("客户端错误: %1")
                            .arg(socket->errorString()));
                    }
                });

        QString info = clientInfo(client);
        ++m_totalClientCount;
        // 更新同时在线客户端峰值
        quint64 currentCount = static_cast<quint64>(m_clients.size());
        if (currentCount > m_peakConnectedClients) {
            m_peakConnectedClients = currentCount;
        }
        emit clientConnected(info);
    }
}

/** @brief 客户端断开回调，从映射表中移除并释放socket */
void TcpServerConnection::onClientDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString info = clientInfo(socket);

    /* socket已断开，descriptor可能无效，遍历查找 */
    qintptr sd = -1;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == socket) {
            sd = it.key();
            break;
        }
    }

    if (sd >= 0) {
        m_clients.remove(sd);
    }
    ++m_totalClientDisconnections;  // 客户端断开计数
    // 更新峰值(移除后当前在线数)
    quint64 currentCount = static_cast<quint64>(m_clients.size());
    if (currentCount > m_peakConnectedClients) {
        m_peakConnectedClients = currentCount;
    }
    emit clientDisconnected(info);
    socket->deleteLater();
}

/** @brief 客户端数据到达回调，发射clientData和dataReceived信号 */
void TcpServerConnection::onClientReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    if (data.isEmpty()) return;

    m_totalRxBytes += data.size();

    QString info = clientInfo(socket);
    emit clientData(info, data);
    /// 同时发射IConnection标准信号，便于上层统一接收
    emit dataReceived(data);
}

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新状态 */
void TcpServerConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/** @brief 获取socket对应的客户端"地址:端口"标识 @param socket 客户端socket @return "地址:端口"格式字符串 */
QString TcpServerConnection::clientInfo(QTcpSocket* socket)
{
    if (!socket) return QString();
    return QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
}
