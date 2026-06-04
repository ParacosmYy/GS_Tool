/**
 * @file TcpServerConnectionClients.cpp
 * @brief TCP服务器 - 客户端管理与统计查询方法实现
 *
 * 本文件从TcpServerConnection.cpp拆分而来，集中管理:
 *   1. 已连接客户端的查询接口 (connectedClients)
 *   2. 客户端广播发送 (broadcastToClients)
 *   3. 运行时统计计数器的读取接口 (totalClientCount/broadcastCount等)
 *   4. 统计计数器重置 (resetStatistics)
 *
 * 拆分目的: 将客户端管理/统计查询与核心连接生命周期(监听/断开/事件回调)
 *           解耦，降低单文件体积，便于独立维护。
 */

#include "connection/tcp/TcpServerConnection.h"

// ---- 客户端查询与管理 ----

/** @brief 获取已连接的客户端列表 @return 客户端"地址:端口"字符串列表 */
QStringList TcpServerConnection::connectedClients() const
{
    QStringList result;
    for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            result.append(clientInfo(socket));
        }
    }
    return result;
}

/** @brief 向所有已连接客户端广播数据 @param data 待广播的字节数据 @return 成功发送的客户端数量 */
int TcpServerConnection::broadcastToClients(const QByteArray& data)
{
    int count = 0;
    int failedCount = 0;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        QTcpSocket* socket = it.value();
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            qint64 written = socket->write(data);
            if (written > 0) {
                m_totalTxBytes += written;
                socket->flush();
                emit bytesWritten(written);
                count++;
            } else {
                ++failedCount;
            }
        }
    }
    if (count > 0) {
        ++m_broadcastCount;
    }
    m_totalFailedSends += static_cast<quint64>(failedCount);  // 累计发送失败次数
    return count;
}

// ---- 统计计数器查询接口 ----

/** @brief 获取历史累计连接客户端总数 @return 客户端连接总数 */
quint64 TcpServerConnection::totalClientCount() const
{
    return m_totalClientCount;
}

/** @brief 获取历史累计断开客户端总数 @return 客户端断开总数 */
quint64 TcpServerConnection::totalClientDisconnections() const
{
    return m_totalClientDisconnections;
}

/** @brief 获取已广播数据包总数 @return 广播次数 */
quint64 TcpServerConnection::broadcastCount() const
{
    return m_broadcastCount;
}

/** @brief 获取累计接收字节数 @return 接收字节总量 */
quint64 TcpServerConnection::totalBytesReceived() const
{
    return m_totalRxBytes;
}

/** @brief 获取累计发送字节数 @return 发送字节总量 */
quint64 TcpServerConnection::totalBytesSent() const
{
    return m_totalTxBytes;
}

/** @brief 获取累计accept错误次数 @return accept错误总数 */
quint64 TcpServerConnection::totalAcceptErrors() const
{
    return m_totalAcceptErrors;
}

/** @brief 重置所有统计计数器为零 */
void TcpServerConnection::resetStatistics()
{
    m_totalClientCount = 0;
    m_totalClientDisconnections = 0;
    m_broadcastCount = 0;
    m_totalRxBytes = 0;
    m_totalTxBytes = 0;
    m_totalAcceptErrors = 0;
    m_totalListenAttempts = 0;
    m_totalWrites = 0;
    m_totalRejectedConnections = 0;
    m_peakConnectedClients = 0;
    m_totalErrors = 0;
    m_totalUnicastSends = 0;
    m_totalFailedSends = 0;
}
