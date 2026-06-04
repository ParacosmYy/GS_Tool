/**
 * @file RttChannelManager.cpp
 * @brief RTT 通道管理器实现 - 多通道数据路由
 *
 * 维护通道号到名称的映射表，将数据分发到对应通道。
 * 同时发出 terminalData（供终端显示）和 channelData（供原始数据消费者）信号。
 */

#include "rtt/RttChannelManager.h"

/** @brief 构造函数，初始化通道管理器 @param parent 父对象 */
RttChannelManager::RttChannelManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 添加一个 RTT 通道 — 注册到映射表，已存在则更新名称 @param channelId 通道编号 @param name 通道名称 */
void RttChannelManager::addChannel(int channelId, const QString& name)
{
    const bool isNew = !m_channels.contains(channelId);
    m_channels.insert(channelId, name);
    if (isNew) {
        ++m_stats.totalChannelCreated;
    }
    m_stats.activeChannels = m_channels.size();
}

/** @brief 移除一个 RTT 通道 — 从映射表中删除 @param channelId 要移除的通道编号 */
void RttChannelManager::removeChannel(int channelId)
{
    if (m_channels.remove(channelId) > 0) {
        ++m_stats.totalChannelRemoved;
    }
    m_stats.activeChannels = m_channels.size();
}

/** @brief 获取所有活跃通道 @return 通道编号到名称的映射表 */
QMap<int, QString> RttChannelManager::activeChannels() const
{
    ++m_stats.channelQueries;
    return m_channels;
}

/** @brief 路由数据到指定通道 — 累计读取统计并发射terminalData和channelData信号 @param channelId 目标通道编号 @param data 待路由的数据 */
void RttChannelManager::routeData(int channelId, const QByteArray& data)
{
    ++m_stats.totalReads;
    ++m_stats.totalDataRouted;
    m_stats.totalBytesRead += static_cast<quint64>(data.size());
    m_stats.totalBytesRouted += static_cast<quint64>(data.size());

    emit terminalData(channelId, data);
    emit channelData(channelId, data);
}

/** @brief 记录一次RTT写操作(数据写入目标设备)，递增写计数和写字节统计 @param channelId 目标通道编号 @param data 写入的数据 */
void RttChannelManager::writeData(int channelId, const QByteArray& data)
{
    ++m_stats.totalWrites;
    m_stats.totalBytesWritten += static_cast<quint64>(data.size());
    emit dataWritten(channelId, data.size());
}

/** @brief 获取累计读操作次数 @return 读操作总次数 */
quint64 RttChannelManager::totalReads() const { return m_stats.totalReads; }

/** @brief 获取累计写操作次数 @return 写操作总次数 */
quint64 RttChannelManager::totalWrites() const { return m_stats.totalWrites; }

/** @brief 获取累计读取字节总数 @return 读取字节总数 */
quint64 RttChannelManager::totalBytesRead() const { return m_stats.totalBytesRead; }

/** @brief 获取累计写入字节总数 @return 写入字节总数 */
quint64 RttChannelManager::totalBytesWritten() const { return m_stats.totalBytesWritten; }

/** @brief 获取累计错误次数 @return 错误总次数 */
quint64 RttChannelManager::errorCount() const { return m_stats.errorCount; }

/** @brief 重置所有统计计数器为初始值 */
void RttChannelManager::resetChannelStatistics()
{
    m_stats = Stats{};
    m_stats.activeChannels = m_channels.size();
}
