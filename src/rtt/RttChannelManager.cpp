/**
 * @file RttChannelManager.cpp
 * @brief RTT 通道管理器实现 — 骨架文件
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
 * @param channelId 通道号
 * @param name 通道名称
 */
void RttChannelManager::addChannel(int channelId, const QString& name)
{
    Q_UNUSED(channelId)
    Q_UNUSED(name)
    // TODO: 注册通道并创建关联的 JLinkRttConnection
}

/**
 * @brief 移除一个 RTT 通道
 * @param channelId 通道号
 */
void RttChannelManager::removeChannel(int channelId)
{
    Q_UNUSED(channelId)
    // TODO: 关闭并销毁对应通道的连接
}

/**
 * @brief 获取所有活跃通道
 * @return 通道号→名称的映射表
 */
QMap<int, QString> RttChannelManager::activeChannels() const
{
    return m_channels;
}

/**
 * @brief 路由数据到指定通道
 *
 * 将数据分发到对应通道的终端显示。
 *
 * @param channelId 目标通道号
 * @param data 数据内容
 */
void RttChannelManager::routeData(int channelId, const QByteArray& data)
{
    Q_UNUSED(channelId)
    Q_UNUSED(data)
    // TODO: 将数据路由到对应通道的终端
}
