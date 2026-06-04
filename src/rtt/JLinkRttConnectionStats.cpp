/**
 * @file JLinkRttConnectionStats.cpp
 * @brief J-Link RTT 连接 - 配置与统计接口实现
 *
 * 从 JLinkRttConnection.cpp 拆分而来，包含通道配置、
 * 参数设置和统计计数器访问/重置方法。
 */

#include "rtt/JLinkRttConnection.h"

/**
 * @brief 配置 RTT 连接参数
 *
 * 解析并存储配置参数。支持 deviceId、interface、speed、channel。
 * 如果参数中包含 channel 键，自动更新通道号。
 *
 * @param params 参数键值对
 */
void JLinkRttConnection::configure(const QVariantMap& params)
{
    m_config = params;

    if (params.contains(QStringLiteral("channel"))) {
        bool ok = false;
        const int ch = params.value(QStringLiteral("channel")).toInt(&ok);
        if (ok && ch >= 0 && ch <= 15) {
            m_channel = ch;
        }
    }
}

/**
 * @brief 设置 RTT 通道号
 * @param ch 通道号（0-15），超出范围不做修改
 */
void JLinkRttConnection::setChannel(int ch)
{
    if (ch >= 0 && ch <= 15) {
        m_channel = ch;
    }
}

/**
 * @brief 获取当前 RTT 通道号
 * @return 通道号
 */
int JLinkRttConnection::channel() const
{
    return m_channel;
}

/** @brief 获取累计读操作次数 @return 读操作总次数 */
quint64 JLinkRttConnection::totalReads() const { return m_totalReads; }

/** @brief 获取累计写操作次数 @return 写操作总次数 */
quint64 JLinkRttConnection::totalWrites() const { return m_totalWrites; }

/** @brief 获取累计读取字节总数 @return 读取字节总数 */
quint64 JLinkRttConnection::totalBytesRead() const { return m_totalBytesRead; }

/** @brief 获取累计写入字节总数 @return 写入字节总数 */
quint64 JLinkRttConnection::totalBytesWritten() const { return m_totalBytesWritten; }

/** @brief 获取累计错误次数 @return 错误总次数 */
quint64 JLinkRttConnection::rttErrorCount() const { return m_errorCount; }

/** @brief 重置 RTT 统计计数器为初始值 */
void JLinkRttConnection::resetRttStatistics()
{
    m_totalReads = 0;
    m_totalWrites = 0;
    m_totalBytesRead = 0;
    m_totalBytesWritten = 0;
    m_errorCount = 0;
}
