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
    m_channels.insert(channelId, name);
}

/** @brief 移除一个 RTT 通道 — 从映射表中删除 @param channelId 要移除的通道编号 */
void RttChannelManager::removeChannel(int channelId)
{
    m_channels.remove(channelId);
}

/** @brief 获取所有活跃通道 @return 通道编号到名称的映射表 */
QMap<int, QString> RttChannelManager::activeChannels() const
{
    return m_channels;
}

/** @brief 路由数据到指定通道 — 累计读取统计并发射terminalData和channelData信号 @param channelId 目标通道编号 @param data 待路由的数据 */
void RttChannelManager::routeData(int channelId, const QByteArray& data)
{
    ++m_totalReads;
    m_totalBytesRead += static_cast<quint64>(data.size());

    emit terminalData(channelId, data);
    emit channelData(channelId, data);
}

/** @brief 获取累计读操作次数 @return 读操作总次数 */
quint64 RttChannelManager::totalReads() const { return m_totalReads; }

/** @brief 获取累计写操作次数 @return 写操作总次数 */
quint64 RttChannelManager::totalWrites() const { return m_totalWrites; }

/** @brief 获取累计读取字节总数 @return 读取字节总数 */
quint64 RttChannelManager::totalBytesRead() const { return m_totalBytesRead; }

/** @brief 获取累计写入字节总数 @return 写入字节总数 */
quint64 RttChannelManager::totalBytesWritten() const { return m_totalBytesWritten; }

/** @brief 获取累计错误次数 @return 错误总次数 */
quint64 RttChannelManager::errorCount() const { return m_errorCount; }

/** @brief 重置所有统计计数器为初始值 */
void RttChannelManager::resetChannelStatistics()
{
    m_totalReads = 0;
    m_totalWrites = 0;
    m_totalBytesRead = 0;
    m_totalBytesWritten = 0;
    m_errorCount = 0;
}
