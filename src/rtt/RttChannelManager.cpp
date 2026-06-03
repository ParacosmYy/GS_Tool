/**
 * @file RttChannelManager.cpp
 * @brief RTT 通道管理器实现 - 多通道数据路由
 *
 * 维护通道号到名称的映射表，将数据分发到对应通道。
 * 同时发出 terminalData（供终端显示）和 channelData（供原始数据消费者）信号。
 */

#include "rtt/RttChannelManager.h"

RttChannelManager::RttChannelManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 添加一个 RTT 通道 — 注册到映射表，已存在则更新名称 */
void RttChannelManager::addChannel(int channelId, const QString& name)
{
    m_channels.insert(channelId, name);
}

/** @brief 移除一个 RTT 通道 — 从映射表中删除 */
void RttChannelManager::removeChannel(int channelId)
{
    m_channels.remove(channelId);
}

/** @brief 获取所有活跃通道 */
QMap<int, QString> RttChannelManager::activeChannels() const
{
    return m_channels;
}

/** @brief 路由数据到指定通道 — 累计读取统计 */
void RttChannelManager::routeData(int channelId, const QByteArray& data)
{
    ++m_totalReads;
    m_totalBytesRead += static_cast<quint64>(data.size());

    emit terminalData(channelId, data);
    emit channelData(channelId, data);
}

quint64 RttChannelManager::totalReads() const { return m_totalReads; }
quint64 RttChannelManager::totalWrites() const { return m_totalWrites; }
quint64 RttChannelManager::totalBytesRead() const { return m_totalBytesRead; }
quint64 RttChannelManager::totalBytesWritten() const { return m_totalBytesWritten; }
quint64 RttChannelManager::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计计数器 */
void RttChannelManager::resetChannelStatistics()
{
    m_totalReads = 0;
    m_totalWrites = 0;
    m_totalBytesRead = 0;
    m_totalBytesWritten = 0;
    m_errorCount = 0;
}
