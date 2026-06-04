/**
 * @file CanConnection.h
 * @brief CAN/CAN-FD连接实现 — 适配器模式，封装CAN适配器串口协议到IConnection接口
 *
 * 职责: 通过串口CAN适配器(CANable/CANtact/LAWICEL兼容)收发CAN帧，
 * 实现IConnection统一接口。支持经典CAN/CAN-FD、位时序、帧过滤、DBC信号解码。
 */
#ifndef CANCONNECTION_H
#define CANCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QElapsedTimer>

/** @brief CAN位时序配置结构体 */
struct CanBitTiming {
    int baudrate = 500000;       ///< 波特率(bps)，默认500kbps
    double samplePoint = 0.875;  ///< 采样点(0.0~1.0)，默认87.5%
    int sjw = 1;                 ///< 同步跳转宽度(SJW)，默认1
};

/** @brief CAN帧过滤器结构体(支持ID掩码+匹配模式) */
struct CanFilter {
    quint32 id = 0;        ///< 匹配ID(标准11位或扩展29位)
    quint32 mask = 0;      ///< 掩码(1=必须匹配位，0=忽略位)
    bool extended = false;  ///< 是否匹配扩展帧
};

class DbcParser;

/** @brief CAN/CAN-FD总线连接实现，封装CAN总线底层通信，通过串口CAN适配器(LAWICEL/SLCAN协议)收发帧 */
class CanConnection : public IConnection {
    Q_OBJECT

public:
    explicit CanConnection(QObject* parent = nullptr); ///< 构造CAN连接
    ~CanConnection() override; ///< 析构函数，自动关闭连接释放DBC解析器

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override;     ///< 固定返回ConnectionType::Can
    QString name() const override;            ///< 获取适配器名称，未配置时返回"未配置"
    ConnectionState state() const override;   ///< 获取当前连接状态
    bool open() override;                     ///< 打开CAN连接，执行LAWICEL初始化序列
    void close() override;                    ///< 关闭CAN连接，发送LAWICEL关闭命令并断开
    qint64 write(const QByteArray& data) override; ///< 向底层串口写入原始数据
    void configure(const QVariantMap& params) override; ///< 配置CAN连接(bitrate/canFd/adapter/samplePoint/sjw/filters)

    // ---- CAN专用接口 ----
    void setBitrate(int bitrate);              ///< 设置波特率(bps)
    void setCanFdEnabled(bool enabled);        ///< 设置CAN-FD模式
    void setBitTiming(const CanBitTiming& timing); ///< 设置位时序配置
    CanBitTiming bitTiming() const;            ///< 获取当前位时序配置
    bool sendFrame(int id, const QByteArray& data, bool extended = false); ///< 发送CAN帧
    bool sendFdFrame(int id, const QByteArray& data, bool extended = false); ///< 发送CAN-FD帧(最长64字节)
    void addFilter(const CanFilter& filter);   ///< 添加帧过滤器
    void clearFilters();                       ///< 清除所有帧过滤器
    QList<CanFilter> filters() const;          ///< 获取帧过滤器列表
    bool acceptsFilter(quint32 id, bool extended) const; ///< 检查帧是否通过过滤器
    void setSerialPort(IConnection* serialPort); ///< 设置底层串口连接(不转移所有权)
    IConnection* serialPort() const;           ///< 获取底层串口连接
    bool loadDbcFile(const QString& filePath); ///< 加载DBC数据库文件
    QMap<QString, double> decodeFrameSignals(quint32 id, const QByteArray& data) const; ///< 解码CAN帧信号值(依赖已加载的DBC)
    DbcParser* dbcParser() const;              ///< 获取DBC解析器实例(未加载时为nullptr)

    // ---- 统计信息接口 ----
    quint64 totalFramesSent() const { return m_totalFramesSent; }      ///< 累计发送帧数
    quint64 totalFramesReceived() const { return m_totalFramesReceived; } ///< 累计接收帧数
    quint64 totalBytesSent() const { return m_totalBytesSent; }        ///< 累计发送字节数
    quint64 totalBytesReceived() const { return m_totalBytesReceived; } ///< 累计接收字节数
    quint64 totalErrors() const { return m_totalErrors; }              ///< 累计错误次数
    quint64 totalStandardFrames() const { return m_totalStandardFrames; } ///< 标准帧计数
    quint64 totalExtendedFrames() const { return m_totalExtendedFrames; } ///< 扩展帧计数
    quint64 totalRtrFrames() const { return m_totalRtrFrames; }        ///< RTR帧计数
    quint64 totalErrorFrames() const { return m_totalErrorFrames; }    ///< 错误帧计数
    quint64 totalFrameErrors() const { return m_totalFrameErrors; }    ///< 帧解析/构建失败计数
    quint64 totalFramesFiltered() const { return m_totalFramesFiltered; } ///< 被过滤器丢弃的帧数
    quint64 totalSignalsDecoded() const { return m_totalSignalsDecoded; } ///< 已解码的信号值总数
    quint64 totalBusOffEvents() const { return m_totalBusOffEvents; }  ///< 累计总线关闭(Bus-Off)事件次数
    quint64 totalFiltersActive() const { return m_totalFiltersActive; } ///< 当前激活的帧过滤器数量
    quint64 totalDroppedFrames() const { return m_totalDroppedFrames; } ///< 累计丢帧数(缓冲区溢出/解析失败)
    quint64 peakFramesPerSec() const { return m_peakFramesPerSec; } ///< 峰值每秒帧数
    void resetStats(); ///< 重置所有统计计数器

signals:
    void frameReceived(int id, const QByteArray& data, bool extended, bool rtr); ///< 收到CAN帧
    void fdFrameReceived(int id, const QByteArray& data, bool extended); ///< 收到CAN-FD帧
    void dbcLoaded(bool success, int messageCount); ///< DBC加载完成

private slots:
    void onSerialDataReceived(const QByteArray& data); ///< 底层串口数据接收槽函数

private:
    qint64 sendCommand(const QString& cmd); ///< 向适配器发送LAWICEL原始命令
    void parseBuffer();                     ///< 解析接收缓冲区中的LAWICEL帧
    static QString bitrateToCommand(int bitrate); ///< 根据波特率获取LAWICEL S命令编号

    CanBitTiming m_bitTiming;          ///< 位时序配置
    bool m_canFdEnabled = false;       ///< CAN-FD模式开关
    QString m_adapterName;             ///< 适配器名称
    ConnectionState m_state = ConnectionState::Disconnected;  ///< 连接状态
    IConnection* m_serialPort = nullptr;  ///< 底层串口连接(不拥有)
    QByteArray m_rxBuffer;             ///< 接收缓冲区
    QList<CanFilter> m_filters;        ///< 帧过滤器列表
    DbcParser* m_dbcParser = nullptr;  ///< DBC解析器(延迟创建)

    // ---- 统计计数器 ----
    quint64 m_totalFramesSent = 0;          ///< 累计发送帧数
    quint64 m_totalFramesReceived = 0;      ///< 累计接收帧数
    quint64 m_totalBytesSent = 0;           ///< 累计发送字节数
    quint64 m_totalBytesReceived = 0;       ///< 累计接收字节数
    quint64 m_totalErrors = 0;              ///< 累计错误次数
    quint64 m_totalStandardFrames = 0;      ///< 标准帧计数
    quint64 m_totalExtendedFrames = 0;      ///< 扩展帧计数
    quint64 m_totalRtrFrames = 0;           ///< RTR帧计数
    quint64 m_totalErrorFrames = 0;         ///< 错误帧计数
    quint64 m_totalFrameErrors = 0;         ///< 帧解析/构建失败计数
    quint64 m_totalFramesFiltered = 0;      ///< 被过滤器丢弃的帧数
    mutable quint64 m_totalSignalsDecoded = 0; ///< 已解码的信号值总数
    mutable quint64 m_totalBusOffEvents = 0; ///< 累计总线关闭事件次数
    mutable quint64 m_totalFiltersActive = 0; ///< 当前激活的帧过滤器数量
    quint64 m_totalDroppedFrames = 0;         ///< 累计丢帧数(缓冲区溢出/解析失败)
    quint64 m_peakFramesPerSec = 0;           ///< 峰值每秒帧数
    quint64 m_lastSecFrameCount = 0;          ///< 当前秒内帧计数(用于计算peakFramesPerSec)
    QElapsedTimer m_peakFpsTimer;             ///< peakFramesPerSec计时器
    qint64 m_lastPeakSec = 0;                 ///< 上一次采样秒数(用于判断秒边界)
};

#endif // CANCONNECTION_H
