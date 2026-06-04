/**
 * @file TcpConnectionStats.cpp
 * @brief TCP连接统计计数器实现 - 统计查询、延迟计算与计数器重置
 *
 * 从 TcpConnection.cpp 拆分而来，包含:
 *   - 各类累计统计的 getter 方法(连接/断开/字节/错误/重连/DNS/KeepAlive)
 *   - 连接建立延迟统计(平均/最大/最近)
 *   - resetStats() 全量重置方法
 */

#include "connection/network/TcpConnection.h"

// ---- 统计计数器实现 ----

/** @brief 获取累计连接成功次数 @return 连接成功总次数 */
quint64 TcpConnection::totalConnections() const { return m_totalConnections; }

/** @brief 获取累计断开连接次数 @return 断开连接总次数 */
quint64 TcpConnection::totalDisconnections() const { return m_totalDisconnections; }

/** @brief 获取累计发送字节数 @return 发送字节总数 */
quint64 TcpConnection::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取累计接收字节数 @return 接收字节总数 */
quint64 TcpConnection::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取累计错误次数 @return 错误总次数 */
quint64 TcpConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计计数器(连接/断开/字节/错误/打开尝试/写入/重连/延迟/DNS/超时/KeepAlive)为零 */
void TcpConnection::resetStats()
{
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalOpenAttempts = 0;
    m_totalWrites = 0;
    m_totalReconnectAttempts = 0;
    m_totalReconnects = 0;
    m_totalDnsLookups = 0;
    m_totalDnsErrors = 0;
    m_totalConnectionTimeouts = 0;
    m_totalKeepAliveProbes = 0;
    m_lastLatencyMs = 0;
    m_maxLatencyMs = 0;
    m_latencySampleCount = 0;
    m_latencySumMs = 0;
    m_isReconnectAttempt = false;
}

/** @brief 获取连接建立平均延迟(毫秒) @return 平均延迟，无采样数据时返回0 */
double TcpConnection::averageLatencyMs() const
{
    if (m_latencySampleCount == 0) { return 0.0; }
    return static_cast<double>(m_latencySumMs) / static_cast<double>(m_latencySampleCount);
}
