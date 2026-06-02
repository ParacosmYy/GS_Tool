/**
 * @file RttChannelManager.cpp
 * @brief RTT 通道管理器实现 — 多通道数据路由
 *
 * 维护通道号到名称的映射表，将数据分发到对应通道。
 * 同时发出 terminalData（供终端显示）和 channelData（供原始数据消费者）信号。
 */

#include "rtt/RttChannelManager.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
RttChannelManager::RttChannelManager(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 添加一个 RTT 通道
 *
 * 将通道号和名称注册到映射表中。
 * 如果通道号已存在，则更新名称。
 *
 * @param channelId 通道号（0-15）
 * @param name 通道名称（如 "Terminal"、"System" 等）
 */
void RttChannelManager::addChannel(int channelId, const QString& name)
{
    m_channels.insert(channelId, name);
}

/**
 * @brief 移除一个 RTT 通道
 *
 * 从映射表中删除指定通道号的条目。
 * 如果通道号不存在则无操作。
 *
 * @param channelId 要移除的通道号
 */
void RttChannelManager::removeChannel(int channelId)
{
    m_channels.remove(channelId);
}

/**
 * @brief 获取所有活跃通道
 *
 * 返回当前已注册的所有通道号→名称映射。
 *
 * @return 通道号→名称的 QMap
 */
QMap<int, QString> RttChannelManager::activeChannels() const
{
    return m_channels;
}

/**
 * @brief 路由数据到指定通道
 *
 * 查找目标通道的名称，然后同时发出两个信号：
 *   - terminalData: 用于终端显示组件
 *   - channelData: 用于原始数据消费者（如日志、协议解析）
 *
 * 即使通道未注册（名称为空），数据仍会被路由。
 *
 * @param channelId 目标通道号
 * @param data 数据内容
 */
void RttChannelManager::routeData(int channelId, const QByteArray& data)
{
    // 无论通道是否已注册，都发出信号
    // 未注册通道的名称为空字符串，接收方可自行处理
    Q_UNUSED(m_channels.value(channelId, QString()))

    emit terminalData(channelId, data);
    emit channelData(channelId, data);
}
