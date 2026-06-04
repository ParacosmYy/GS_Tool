/**
 * @file UdpMulticastGroup.cpp
 * @brief UDP组播连接 - 组播组管理与统计接口实现
 *
 * 从 UdpMulticastConnection.cpp 拆分而来，包含组播组加入/离开、
 * 网络接口设置和所有统计getter/resetStatistics方法。
 */

#include "connection/tcp/UdpMulticastConnection.h"
#include <QNetworkInterface>

/** @brief 加入指定组播组 @param groupAddress 组播地址 */
void UdpMulticastConnection::joinGroup(const QHostAddress& groupAddress)
{
    if (!m_socket) return;

    ++m_totalJoins;
    if (m_usingCustomInterface && m_multicastInterface.isValid()) {
        if (!m_socket->joinMulticastGroup(groupAddress, m_multicastInterface)) {
            emit errorOccurred(tr("加入组播组失败: %1").arg(m_socket->errorString()));
        }
    } else {
        if (!m_socket->joinMulticastGroup(groupAddress)) {
            emit errorOccurred(tr("加入组播组失败: %1").arg(m_socket->errorString()));
        }
    }
}

/** @brief 离开指定组播组 @param groupAddress 组播地址 */
void UdpMulticastConnection::leaveGroup(const QHostAddress& groupAddress)
{
    if (!m_socket) return;

    ++m_totalLeaves;
    if (m_usingCustomInterface && m_multicastInterface.isValid()) {
        m_socket->leaveMulticastGroup(groupAddress, m_multicastInterface);
    } else {
        m_socket->leaveMulticastGroup(groupAddress);
    }
}

/** @brief 设置组播数据发送使用的网络接口 @param interfaceName 网络接口名称 */
void UdpMulticastConnection::setMulticastInterface(const QString& interfaceName)
{
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.humanReadableName() == interfaceName) {
            m_multicastInterface = iface;
            m_usingCustomInterface = true;
            return;
        }
    }
    m_usingCustomInterface = false;
}

/** @brief 获取已发送数据报计数 @return 发送数据报总数 */
quint64 UdpMulticastConnection::datagramsSent() const
{
    return m_dgramsSent;
}

/** @brief 获取已接收数据报计数 @return 接收数据报总数 */
quint64 UdpMulticastConnection::datagramsReceived() const
{
    return m_dgramsRecv;
}

/** @brief 获取累计发送字节数 @return 发送字节总量 */
qint64 UdpMulticastConnection::totalBytesSent() const
{
    return m_txBytes;
}

/** @brief 获取累计接收字节数 @return 接收字节总量 */
qint64 UdpMulticastConnection::totalBytesReceived() const
{
    return m_rxBytes;
}

/** @brief 重置统计数据(数据报/字节/加入离开次数/错误/打开次数/写入错误/组播/单播数据报)为零 */
void UdpMulticastConnection::resetStatistics()
{
    m_dgramsSent = 0;
    m_dgramsRecv = 0;
    m_txBytes = 0;
    m_rxBytes = 0;
    m_totalJoins = 0;
    m_totalLeaves = 0;
    m_totalNetworkErrors = 0;
    m_totalOpenAttempts = 0;
    m_totalWriteErrors = 0;
    m_totalGroupDatagrams = 0;
    m_totalPeerDatagrams = 0;
}

/** @brief 获取组播组加入总次数 @return 加入组播组总次数 */
quint64 UdpMulticastConnection::totalJoins() const
{
    return m_totalJoins;
}

/** @brief 获取组播组离开总次数 @return 离开组播组总次数 */
quint64 UdpMulticastConnection::totalLeaves() const
{
    return m_totalLeaves;
}
