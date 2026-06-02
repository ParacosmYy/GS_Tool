/**
 * @file RttChannelManager.h
 * @brief RTT 通道管理器 — 管理多个 RTT 通道的数据路由
 *
 * 负责创建/销毁 RTT 通道，并将接收到的数据分发到对应的通道。
 * 每个通道有独立的名称标识，支持动态添加和移除。
 *
 * 协作关系:
 *   - JLinkRttConnection: 提供底层 RTT 通道通信
 *   - RttConfigPanel: 通过配置创建通道
 *   - TerminalWidget: 接收通道数据显示
 */
#ifndef RTTCHANNELMANAGER_H
#define RTTCHANNELMANAGER_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QString>

/**
 * @brief RTT 通道管理器
 *
 * 管理 RTT 通道的注册/注销和数据路由。
 * 维护通道号到通道名称的映射表，支持多通道并行数据分发。
 */
class RttChannelManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit RttChannelManager(QObject* parent = nullptr);

    /**
     * @brief 添加一个 RTT 通道
     * @param channelId 通道号
     * @param name 通道名称
     */
    void addChannel(int channelId, const QString& name);

    /**
     * @brief 移除一个 RTT 通道
     * @param channelId 通道号
     */
    void removeChannel(int channelId);

    /**
     * @brief 获取所有活跃通道
     * @return 通道号→名称的映射表
     */
    QMap<int, QString> activeChannels() const;

    /**
     * @brief 路由数据到指定通道
     *
     * 查找通道名称后同时发出 terminalData 和 channelData 信号。
     * 未知通道的数据仍会被路由（名称为空字符串）。
     *
     * @param channelId 目标通道号
     * @param data 数据内容
     */
    void routeData(int channelId, const QByteArray& data);

signals:
    /**
     * @brief 通道数据已路由到终端
     * @param channelId 通道号
     * @param data 数据内容
     */
    void terminalData(int channelId, const QByteArray& data);

    /**
     * @brief 原始通道数据信号
     * @param channelId 通道号
     * @param data 数据内容
     */
    void channelData(int channelId, const QByteArray& data);

private:
    QMap<int, QString> m_channels;  ///< 通道号→名称映射表
};

#endif // RTTCHANNELMANAGER_H
