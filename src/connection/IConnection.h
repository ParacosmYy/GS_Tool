#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QVariant>
#include "core/Constants.h"

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

signals:
    /** @brief 收到数据时发出 */
    void dataReceived(const QByteArray& data);

    /** @brief 连接状态变化时发出 */
    void stateChanged(ConnectionState newState);

    /** @brief 发生错误时发出 */
    void errorOccurred(const QString& errorMsg);
};

#endif // ICONNECTION_H
