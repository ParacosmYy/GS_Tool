/**
 * @file TcpConnectionHandlers.cpp
 * @brief TCP连接Socket事件处理器实现 - 连接/断开/数据/错误/新连接回调
 *
 * 从 TcpConnection.cpp 拆分而来，包含:
 *   - onSocketConnected: 客户端连接成功回调(延迟计算/重连计数/KeepAlive)
 *   - onSocketDisconnected: 连接断开回调(断开计数/状态更新)
 *   - onSocketReadyRead: 数据到达回调(读取转发/接收字节统计)
 *   - onSocketError: 网络错误回调(错误计数/DNS错误/中文诊断翻译)
 *   - onNewConnection: 服务端新客户端连接回调(旧客户端替换/信号连接)
 */

#include "connection/network/TcpConnection.h"

// ---- Socket事件处理器 ----

/** @brief 客户端模式socket连接成功回调，停止超时定时器，计算连接延迟并更新状态为Connected */
void TcpConnection::onSocketConnected()
{
    // 连接成功，取消超时定时器
    if (m_connectTimer) m_connectTimer->stop();

    // 计算连接建立延迟(从connectToHost到connected回调)
    if (m_connectStartTime.isValid()) {
        qint64 latency = m_connectStartTime.elapsed();
        m_lastLatencyMs = latency;
        if (latency > m_maxLatencyMs) {
            m_maxLatencyMs = latency;
        }
        m_latencySumMs += latency;
        ++m_latencySampleCount;
        m_connectStartTime.invalidate();
    }

    ++m_totalConnections;  // 客户端连接成功计数

    // 如果是重连尝试，额外计数重连成功次数
    if (m_isReconnectAttempt) {
        ++m_totalReconnects;
        m_isReconnectAttempt = false;
    }

    ++m_totalKeepAliveProbes;  // 连接成功后KeepAlive生效

    updateState(ConnectionState::Connected);
}

/** @brief 客户端模式socket断开回调，更新状态为Disconnected并递增断开计数 */
void TcpConnection::onSocketDisconnected()
{
    ++m_totalDisconnections;  // 对端断开计数
    updateState(ConnectionState::Disconnected);
}

/** @brief socket可读回调，读取全部数据并发射dataReceived信号，同时累计接收字节统计 */
void TcpConnection::onSocketReadyRead()
{
    QTcpSocket* senderSock = qobject_cast<QTcpSocket*>(sender());
    if (!senderSock) return;

    QByteArray data = senderSock->readAll();
    if (!data.isEmpty()) {
        m_totalBytesReceived += static_cast<quint64>(data.size());  // 累计接收字节
        emit dataReceived(data);
    }
}

/** @brief socket错误回调，翻译错误码为中文诊断信息并发射errorOccurred信号 @param error Qt网络错误枚举 */
void TcpConnection::onSocketError(QAbstractSocket::SocketError error)
{
    ++m_errorCount;  // 网络错误计数
    if (error == QAbstractSocket::HostNotFoundError) {
        ++m_totalDnsErrors;  // DNS解析失败计数
    }
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) {
        emit errorOccurred(translateNetworkError(error, QString()));
        updateState(ConnectionState::Error);
        return;
    }
    QString systemError = sock->errorString();
    emit errorOccurred(translateNetworkError(error, systemError));
    updateState(ConnectionState::Error);
}

/** @brief 服务端模式新客户端连接回调，替换旧客户端socket并连接readyRead/disconnected/errorOccurred信号 */
void TcpConnection::onNewConnection()
{
    if (m_clientSocket) {
        // 断开旧客户端的所有信号连接，防止 disconnected lambda 在新客户端赋值后删除错误的 socket
        disconnect(m_clientSocket, nullptr, this, nullptr);
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        ++m_totalDisconnections;  // 旧客户端被替换计为断开
    }

    m_clientSocket = m_server->nextPendingConnection();
    if (m_clientSocket) {
        ++m_totalConnections;  // 服务端接受新连接计数
        QTcpSocket* sock = m_clientSocket;  // 捕获当前 socket 指针，防止 lambda 通过 m_clientSocket 访问到新 socket
        connect(sock, &QTcpSocket::readyRead,
                this, &TcpConnection::onSocketReadyRead);
        connect(sock, &QTcpSocket::disconnected,
                this, [this, sock]() {
                    // 仅当 m_clientSocket 仍指向本 socket 时才清理（新连接已替换则跳过）
                    if (m_clientSocket == sock) {
                        m_clientSocket->deleteLater();
                        m_clientSocket = nullptr;
                    }
                    ++m_totalDisconnections;  // 客户端主动断开计数
                });
        connect(sock, &QTcpSocket::errorOccurred,
                this, &TcpConnection::onSocketError);
        updateState(ConnectionState::Connected);
    }
}
