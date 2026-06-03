/**
 * @file JLinkRttConnection.h
 * @brief J-Link RTT 连接实现 — 通过 SEGGER RTT 协议与目标 MCU 通信
 *
 * 实现 IConnection 接口，使用 J-Link SDK 建立 RTT 通道连接。
 * 支持多通道数据收发，每个通道独立读写。
 *
 * 协作关系:
 *   - JLinkSdkLoader: 加载 J-Link SDK 动态库
 *   - RttChannelManager: 管理多通道数据路由
 *   - ConnectionFactory: 按 ConnectionType::Rtt 创建此实例
 */
#ifndef JLINKRTTCONNECTION_H
#define JLINKRTTCONNECTION_H

#include "connection/interface/IConnection.h"

class JLinkSdkLoader;

/**
 * @brief J-Link RTT 连接类
 *
 * 通过 SEGGER J-Link 调试器与目标 MCU 的 RTT 缓冲区通信。
 * 每个 JLinkRttConnection 实例绑定一个 RTT 通道。
 */
class JLinkRttConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit JLinkRttConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动关闭连接 */
    ~JLinkRttConnection() override;

    /** @brief 获取连接类型（固定返回 ConnectionType::Rtt） */
    ConnectionType type() const override;

    /** @brief 获取连接名称，格式 "RTT CH<通道号>" */
    QString name() const override;

    /** @brief 获取当前连接状态 */
    ConnectionState state() const override;

    /** @brief 打开 RTT 连接（连接 J-Link 并启动 RTT） */
    bool open() override;

    /** @brief 关闭 RTT 连接 */
    void close() override;

    /**
     * @brief 向 RTT 通道写入数据
     * @param data 要发送的字节数据
     * @return 实际写入字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 配置 RTT 连接参数
     *
     * 支持的参数键:
     *   - deviceId: 目标设备标识
     *   - interface: 调试接口 (JTAG/SWD)
     *   - speed: 连接速度 (kHz)
     *   - channel: RTT 通道号
     *
     * @param params 参数键值对
     */
    void configure(const QVariantMap& params) override;

    /**
     * @brief 设置 RTT 通道号
     * @param ch 通道号（0-15）
     */
    void setChannel(int ch);

    /** @brief 获取当前 RTT 通道号 */
    int channel() const;

    // ---- RTT 统计 getter ----

    /** @brief 获取累计读操作次数 */
    quint64 totalReads() const;

    /** @brief 获取累计写操作次数 */
    quint64 totalWrites() const;

    /** @brief 获取累计读取字节总数 */
    quint64 totalBytesRead() const;

    /** @brief 获取累计写入字节总数 */
    quint64 totalBytesWritten() const;

    /** @brief 获取累计错误次数 */
    quint64 rttErrorCount() const;

    /** @brief 重置 RTT 统计计数器为初始值 */
    void resetRttStatistics();

private:
    int m_channel = 0;                                      ///< RTT 通道号
    ConnectionState m_state = ConnectionState::Disconnected; ///< 连接状态
    QVariantMap m_config;                                   ///< 当前配置参数
    JLinkSdkLoader* m_sdkLoader = nullptr;                  ///< SDK 加载器单例引用

    // RTT 统计计数器
    quint64 m_totalReads = 0;           ///< 累计读操作次数
    quint64 m_totalWrites = 0;          ///< 累计写操作次数
    quint64 m_totalBytesRead = 0;       ///< 累计读取字节总数
    quint64 m_totalBytesWritten = 0;    ///< 累计写入字节总数
    quint64 m_errorCount = 0;           ///< 累计错误次数
};

#endif // JLINKRTTCONNECTION_H
