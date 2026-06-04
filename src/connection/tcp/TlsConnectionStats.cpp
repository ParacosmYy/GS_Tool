/**
 * @file TlsConnectionStats.cpp
 * @brief TLS连接统计查询和重置方法实现
 *
 * 从 TlsConnection.cpp 拆分而来，包含SSL握手/字节/错误
 * 统计getter和resetStats方法。
 */

#include "connection/tcp/TlsConnection.h"

/** @brief 获取SSL握手完成次数 @return 握手成功总数 */
quint64 TlsConnection::totalHandshakes() const { return m_totalHandshakes; }

/** @brief 获取已发送字节总数 @return 发送字节总量 */
quint64 TlsConnection::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取已接收字节总数 @return 接收字节总量 */
quint64 TlsConnection::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取错误计数 @return 错误总数 */
quint64 TlsConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有TLS统计数据(握手/字节/错误/断开/证书/超时计数)为零 */
void TlsConnection::resetStats()
{
    m_totalHandshakes = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalOpens = 0;
    m_totalCloses = 0;
    m_totalWrites = 0;
    m_totalSslErrors = 0;
    m_totalDisconnections = 0;
    m_totalCertificateLoads = 0;
    m_totalConnectionTimeouts = 0;
}
