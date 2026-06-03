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

/**
 * @brief CAN/CAN-FD总线连接实现
 *
 * 封装CAN总线底层通信，通过串口CAN适配器(LAWICEL/SLCAN协议)收发帧。
 * 支持经典CAN(8字节)和CAN-FD(64字节)模式，可配置波特率、位时序、帧过滤。
 */
class CanConnection : public IConnection {
    Q_OBJECT

public:
    /** @brief 构造CAN连接 @param parent 父对象 */
    explicit CanConnection(QObject* parent = nullptr);
    /** @brief 析构函数，自动关闭连接释放DBC解析器 */
    ~CanConnection() override;

    // ---- IConnection 接口实现 ----
    ConnectionType type() const override;
    QString name() const override;
    ConnectionState state() const override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    /** @brief 配置CAN连接(支持bitrate/canFd/adapter/samplePoint/sjw/filters) */
    void configure(const QVariantMap& params) override;

    // ---- CAN专用接口 ----
    /** @brief 设置波特率 @param bitrate 波特率(bps) */
    void setBitrate(int bitrate);
    /** @brief 设置CAN-FD模式 @param enabled true启用 */
    void setCanFdEnabled(bool enabled);
    /** @brief 设置位时序配置 @param timing 位时序参数 */
    void setBitTiming(const CanBitTiming& timing);
    /** @brief 获取当前位时序配置 */
    CanBitTiming bitTiming() const;
    /** @brief 发送CAN帧 @param id 帧ID @param data 帧数据 @param extended 扩展帧 @return 成功返回true */
    bool sendFrame(int id, const QByteArray& data, bool extended = false);
    /** @brief 发送CAN-FD帧(最长64字节) @return 成功返回true */
    bool sendFdFrame(int id, const QByteArray& data, bool extended = false);
    /** @brief 添加帧过滤器 @param filter 过滤器 */
    void addFilter(const CanFilter& filter);
    /** @brief 清除所有帧过滤器 */
    void clearFilters();
    /** @brief 获取帧过滤器列表 */
    QList<CanFilter> filters() const;
    /** @brief 检查帧是否通过过滤器 @return true=通过 */
    bool acceptsFilter(quint32 id, bool extended) const;
    /** @brief 设置底层串口连接 @param serialPort 串口连接(不转移所有权) */
    void setSerialPort(IConnection* serialPort);
    /** @brief 获取底层串口连接 */
    IConnection* serialPort() const;
    /** @brief 加载DBC数据库文件 @param filePath DBC文件路径 @return 成功返回true */
    bool loadDbcFile(const QString& filePath);
    /** @brief 解码CAN帧信号值(依赖已加载的DBC) @return 信号名→物理值映射 */
    QMap<QString, double> decodeFrameSignals(quint32 id, const QByteArray& data) const;
    /** @brief 获取DBC解析器实例(未加载时为nullptr) */
    DbcParser* dbcParser() const;

    // ---- 统计信息接口 ----
    quint64 totalFramesSent() const { return m_totalFramesSent; }
    quint64 totalFramesReceived() const { return m_totalFramesReceived; }
    quint64 totalBytesSent() const { return m_totalBytesSent; }
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }
    quint64 totalErrors() const { return m_totalErrors; }
    quint64 totalStandardFrames() const { return m_totalStandardFrames; }
    quint64 totalExtendedFrames() const { return m_totalExtendedFrames; }
    quint64 totalRtrFrames() const { return m_totalRtrFrames; }
    quint64 totalErrorFrames() const { return m_totalErrorFrames; }
    quint64 totalFramesFiltered() const { return m_totalFramesFiltered; }
    quint64 totalSignalsDecoded() const { return m_totalSignalsDecoded; }
    void resetStats();

signals:
    /** @brief 收到CAN帧 @param id 帧ID @param data 帧数据 @param extended 扩展帧 @param rtr 远程帧 */
    void frameReceived(int id, const QByteArray& data, bool extended, bool rtr);
    /** @brief 收到CAN-FD帧 @param id 帧ID @param data 帧数据(最长64字节) */
    void fdFrameReceived(int id, const QByteArray& data, bool extended);
    /** @brief DBC加载完成 @param success 是否成功 @param messageCount 消息数量 */
    void dbcLoaded(bool success, int messageCount);

private slots:
    void onSerialDataReceived(const QByteArray& data);

private:
    qint64 sendCommand(const QString& cmd);
    void parseBuffer();
    static QString bitrateToCommand(int bitrate);

    CanBitTiming m_bitTiming;          ///< 位时序配置
    bool m_canFdEnabled = false;       ///< CAN-FD模式开关
    QString m_adapterName;             ///< 适配器名称
    ConnectionState m_state = ConnectionState::Disconnected;  ///< 连接状态
    IConnection* m_serialPort = nullptr;  ///< 底层串口连接(不拥有)
    QByteArray m_rxBuffer;             ///< 接收缓冲区
    QList<CanFilter> m_filters;        ///< 帧过滤器列表
    DbcParser* m_dbcParser = nullptr;  ///< DBC解析器(延迟创建)

    // ---- 统计计数器 ----
    quint64 m_totalFramesSent = 0;
    quint64 m_totalFramesReceived = 0;
    quint64 m_totalBytesSent = 0;
    quint64 m_totalBytesReceived = 0;
    quint64 m_totalErrors = 0;
    quint64 m_totalStandardFrames = 0;
    quint64 m_totalExtendedFrames = 0;
    quint64 m_totalRtrFrames = 0;
    quint64 m_totalErrorFrames = 0;
    quint64 m_totalFramesFiltered = 0;
    mutable quint64 m_totalSignalsDecoded = 0;
};

#endif // CANCONNECTION_H
