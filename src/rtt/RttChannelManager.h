/**
 * @file RttChannelManager.h
 * @brief RTT 通道管理器 - 管理多个 RTT 通道的数据路由
 *
 * 负责创建/销毁 RTT 通道，并将接收到的数据分发到对应的通道。
 * 每个通道有独立的名称标识，支持动态添加和移除。
 *
 * 协作关系:
 *   - JLinkRttConnection: 通过 routeData() 分发 RTT 数据
 *   - TerminalWidget: 监听 terminalData 信号显示数据
 *   - RttConfigPanel: 提供 UI 管理通道配置
 */
#ifndef RTTCHANNELMANAGER_H
#define RTTCHANNELMANAGER_H

#include <QObject>
#include <QMap>

/**
 * @brief RTT 通道管理器
 *
 * 使用定时器周期性轮询 IConnection 的 pinoutSignals() 接口，
 * 当信号线状态发生变化时发出通知。
 */
class RttChannelManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit RttChannelManager(QObject* parent = nullptr);

    /** @brief 添加一个 RTT 通道 @param channelId 通道编号(0-15) @param name 通道显示名称 */
    void addChannel(int channelId, const QString& name);

    /** @brief 移除一个 RTT 通道 @param channelId 要移除的通道编号 */
    void removeChannel(int channelId);

    /** @brief 获取所有活跃通道 @return 通道编号→名称的映射表 */
    QMap<int, QString> activeChannels() const;

    /** @brief 路由数据到指定通道 @param channelId 目标通道编号 @param data 要路由的字节数据 */
    void routeData(int channelId, const QByteArray& data);

    /** @brief 获取累计读取次数 @return 读取总次数 */
    quint64 totalReads() const;
    /** @brief 获取累计写入次数 @return 写入总次数 */
    quint64 totalWrites() const;
    /** @brief 获取累计读取字节数 @return 读取字节总数 */
    quint64 totalBytesRead() const;
    /** @brief 获取累计写入字节数 @return 写入字节总数 */
    quint64 totalBytesWritten() const;
    /** @brief 获取累计错误次数 @return 错误总次数 */
    quint64 errorCount() const;
    /** @brief 重置所有统计计数器 */
    void resetChannelStatistics();

signals:
    /** @brief 通道数据已路由到终端 */
    void terminalData(int channelId, const QByteArray& data);
    /** @brief 通道原始数据（供日志/协议解析使用） */
    void channelData(int channelId, const QByteArray& data);

private:
    QMap<int, QString> m_channels;  ///< 通道号->名称映射表

    quint64 m_totalReads = 0;        ///< 累计读取次数
    quint64 m_totalWrites = 0;       ///< 累计写入次数
    quint64 m_totalBytesRead = 0;    ///< 累计读取字节数
    quint64 m_totalBytesWritten = 0; ///< 累计写入字节数
    quint64 m_errorCount = 0;        ///< 累计错误次数
};

#endif // RTTCHANNELMANAGER_H
