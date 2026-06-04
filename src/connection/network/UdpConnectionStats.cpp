/**
 * @file UdpConnectionStats.cpp
 * @brief UDP连接统计查询和多播组管理实现
 *
 * 从 UdpConnection.cpp 拆分而来，包含统计getter、
 * 多播组加入/离开和resetStats方法。
 */

#include "connection/network/UdpConnection.h"

#include <QUdpSocket>
#include <QHostAddress>

/** @brief 加入指定的多播组 @param groupAddr 多播组地址(如"239.0.0.1") @return true=加入成功, false=socket无效或加入失败 */
bool UdpConnection::joinMulticastGroup(const QString& groupAddr)
{
    if (!m_socket) { return false; }
    QHostAddress addr(groupAddr);
    if (!addr.isMulticast()) { return false; }
    bool ok = m_socket->joinMulticastGroup(addr);
    if (ok) {
        ++m_totalMulticastJoins;
    }
    return ok;
}

/** @brief 离开指定的多播组 @param groupAddr 多播组地址 @return true=离开成功, false=socket无效或离开失败 */
bool UdpConnection::leaveMulticastGroup(const QString& groupAddr)
{
    if (!m_socket) { return false; }
    QHostAddress addr(groupAddr);
    if (!addr.isMulticast()) { return false; }
    bool ok = m_socket->leaveMulticastGroup(addr);
    if (ok) {
        ++m_totalMulticastLeaves;
    }
    return ok;
}

// ---- 统计接口 ----

/** @brief 获取已发送数据报总数 @return 数据报发送总数 */
quint64 UdpConnection::totalDatagramsSent() const { return m_totalDatagramsSent; }

/** @brief 获取已接收数据报总数 @return 数据报接收总数 */
quint64 UdpConnection::totalDatagramsReceived() const { return m_totalDatagramsReceived; }

/** @brief 获取已发送字节总数 @return 发送字节总数 */
quint64 UdpConnection::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取已接收字节总数 @return 接收字节总数 */
quint64 UdpConnection::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取错误计数 @return 累计错误次数 */
quint64 UdpConnection::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计数据(数据报/字节/错误/广播/Socket错误/多播计数)为零 */
void UdpConnection::resetStats()
{
    m_totalDatagramsSent = 0;
    m_totalDatagramsReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalDatagramErrors = 0;
    m_totalBroadcastsSent = 0;
    m_totalSocketErrors = 0;
    m_totalMulticastJoins = 0;
    m_totalMulticastLeaves = 0;
}
