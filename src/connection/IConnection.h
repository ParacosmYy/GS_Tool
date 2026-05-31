/**
 * @file IConnection.h
 * @brief 连接抽象接口 - 所有连接类型(串口/TCP/UDP/RTT)的统一协议
 *
 * 定义了连接的抽象基类和通用数据结构(PinoutSignals, SerialErrorCounters)。
 * 上层功能通过此接口与底层通信，无需关心具体连接方式。
 */
#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QVariant>
#include "core/Constants.h"

/**
 * @brief 串口信号线状态结构体
 * 包含6个标准信号线的当前电平状态(true=HIGH, false=LOW)
 */
struct PinoutSignals {
    bool cts = false;  ///< Clear To Send (输入)
    bool dsr = false;  ///< Data Set Ready (输入)
    bool dcd = false;  ///< Data Carrier Detect (输入)
    bool ri = false;   ///< Ring Indicator (输入)
    bool dtr = false;  ///< Data Terminal Ready (输出)
    bool rts = false;  ///< Request To Send (输出)
};

/**
 * @brief 串口通信错误计数器结构体
 * 记录各类通信错误的累计次数，用于诊断连接质量
 */
struct SerialErrorCounters {
    int framingErrors = 0;  ///< 帧错误计数（起始位/停止位不匹配）
    int parityErrors = 0;   ///< 校验错误计数（奇偶校验失败）
    int overrunErrors = 0;  ///< 溢出错误计数（接收缓冲区满导致数据丢失）
    int unknownErrors = 0;  ///< 未知错误计数（无法分类的通信错误）
};

/**
 * @brief 连接抽象接口 - 所有连接类型(串口/TCP/UDP/RTT)的统一协议
 *
 * 上层功能(终端/协议解析/OTA)通过此接口与底层通信，无需关心具体连接方式。
 * 新增连接类型只需实现此接口并注册到 ConnectionFactory。
 *
 * 协作关系:
 *   - ConnectionFactory: 按类型创建 IConnection 实例
 *   - ConnectionController: 管理 IConnection 的生命周期
 *   - SendController/OtaManager: 通过 IConnection 读写数据
 *
 * 设计模式:
 *   - 策略模式: 不同连接类型作为可互换的策略
 *   - 工厂模式: ConnectionFactory 根据 ConnectionType 创建具体实例
 */
class IConnection : public QObject {
    Q_OBJECT

public:
    explicit IConnection(QObject* parent = nullptr)
        : QObject(parent) {}

    virtual ~IConnection() = default;

    /** @brief 获取连接类型 */
    virtual ConnectionType type() const = 0;

    /** @brief 获取连接名称 (如 "COM3" / "TCP:192.168.1.100:8080") */
    virtual QString name() const = 0;

    /** @brief 获取当前连接状态 */
    virtual ConnectionState state() const = 0;

    /** @brief 打开连接，返回是否成功 */
    virtual bool open() = 0;

    /** @brief 关闭连接 */
    virtual void close() = 0;

    /**
     * @brief 发送数据
     * @param data 要发送的字节数据
     * @return 实际发送的字节数，-1表示失败
     */
    virtual qint64 write(const QByteArray& data) = 0;

    /**
     * @brief 通过参数映射配置连接
     *
     * 子类自行解析自己需要的参数，忽略不认识的key:
     * - 串口: portName, baudRate, dataBits, parity, stopBits, flowControl, dtr, rts
     * - TCP: host, port, mode
     * - UDP: localPort, remoteHost, remotePort
     *
     * @param params 参数键值对
     */
    virtual void configure(const QVariantMap& params) = 0;

    // ---- 线路信号控制（仅串口连接有效，其他类型为空实现） ----

    /**
     * @brief 控制数据终端就绪 (DTR) 信号线
     * 默认空实现，仅 SerialConnection 重写
     * @param enabled true=拉高 DTR, false=拉低 DTR
     */
    virtual void setDtr(bool enabled) { Q_UNUSED(enabled); }

    /**
     * @brief 控制请求发送 (RTS) 信号线
     * 默认空实现，仅 SerialConnection 重写
     * @param enabled true=拉高 RTS, false=拉低 RTS
     */
    virtual void setRts(bool enabled) { Q_UNUSED(enabled); }

    /**
     * @brief 查询 DTR 信号线当前状态
     * @return true=DTR 已拉高, false=DTR 已拉低; 非串口连接始终返回 false
     */
    virtual bool isDtr() const { return false; }

    /**
     * @brief 查询 RTS 信号线当前状态
     * @return true=RTS 已拉高, false=RTS 已拉低; 非串口连接始终返回 false
     */
    virtual bool isRts() const { return false; }

    /**
     * @brief 发送Break信号(部分bootloader需要)
     * @param duration Break持续时间(毫秒)，默认100ms
     */
    virtual void sendBreak(int duration = 100) { Q_UNUSED(duration); }

    /** @brief 查询当前信号线电平状态(CTS/DSR/DCD/RI/DTR/RTS)
     *  默认返回全false(无信号); SerialConnection覆盖实现
     */
    virtual PinoutSignals pinoutSignals() const { return {}; }

    /**
     * @brief 查询通信错误计数器
     * 默认返回全零; SerialConnection覆盖实现
     * @return 各类错误的累计计数
     */
    virtual SerialErrorCounters errorCounters() const { return {}; }

signals:
    /** @brief 收到数据时发出 */
    void dataReceived(const QByteArray& data);

    /** @brief 连接状态变化时发出 */
    void stateChanged(ConnectionState newState);

    /** @brief 发生错误时发出 */
    void errorOccurred(const QString& errorMsg);

    /**
     * @brief 数据已写入底层传输通道信号
     * @param bytes 实际写入的字节数
     */
    void bytesWritten(qint64 bytes);
};

Q_DECLARE_METATYPE(PinoutSignals)
Q_DECLARE_METATYPE(SerialErrorCounters)

#endif // ICONNECTION_H
