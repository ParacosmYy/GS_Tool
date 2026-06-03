/**
 * @file ModbusMaster.h
 * @brief Modbus主站（客户端）— 发起请求并处理响应
 *
 * 通过IConnection接口发送Modbus请求帧，接收并解析响应。
 * 支持RTU/ASCII/TCP模式，提供标准功能码便捷方法和自定义帧发送。
 * 统计增强: 请求数/响应数/超时数/错误数独立计数，支持统一重置。
 */
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <QObject>
#include <QTimer>
#include "protocol/modbus/ModbusTypes.h"
#include "connection/interface/IConnection.h"

/**
 * @brief Modbus主站控制器
 * 负责构造请求帧、发送并等待响应、超时管理。
 * 提供运行时统计接口用于诊断和监控。
 */
class ModbusMaster : public QObject {
    Q_OBJECT

public:
    explicit ModbusMaster(QObject* parent = nullptr);

    /** @brief 设置底层连接 */
    void setConnection(IConnection* connection);

    /** @brief 设置响应超时时间（毫秒） */
    void setTimeout(int ms);

    /**
     * @brief 读线圈状态 (FC01)
     * @param slave 从站地址
     * @param start 起始地址
     * @param count 读取数量
     * @return 请求是否已成功发送
     */
    bool readCoils(int slave, int start, int count);

    /**
     * @brief 读保持/输入寄存器 (FC03/FC04)
     * @param slave 从站地址
     * @param start 起始地址
     * @param count 读取数量
     * @return 请求是否已成功发送
     */
    bool readRegisters(int slave, int start, int count);

    /**
     * @brief 写单个寄存器 (FC06)
     * @param slave 从站地址
     * @param addr 寄存器地址
     * @param value 写入值
     * @return 请求是否已成功发送
     */
    bool writeSingleRegister(int slave, int addr, quint16 value);

    /**
     * @brief 写多个寄存器 (FC16)
     * @param slave 从站地址
     * @param addr 起始地址
     * @param values 写入值列表
     * @return 请求是否已成功发送
     */
    bool writeMultipleRegisters(int slave, int addr, const QList<quint16>& values);

    /**
     * @brief 发送自定义Modbus帧
     * @param frame 自定义帧数据
     * @return 请求是否已成功发送
     */
    bool sendCustomFrame(const ModbusFrame& frame);

    // ---- 统计接口 ----

    /** @brief 获取累计发送的请求总数 */
    quint64 totalRequests() const;

    /** @brief 获取累计接收的有效响应总数 */
    quint64 totalResponses() const;

    /** @brief 获取累计超时次数 */
    quint64 totalTimeouts() const;

    /** @brief 获取累计Modbus异常响应总数 */
    quint64 totalErrors() const;

    /** @brief 重置所有统计计数器 */
    void resetStats();

    // ---- 兼容旧接口 ----

    /** @brief 获取已发送请求计数（兼容旧接口，等同 totalRequests） */
    quint64 requestCount() const;

    /** @brief 获取已接收响应计数（兼容旧接口，等同 totalResponses） */
    quint64 responseCount() const;

    /** @brief 获取超时次数（兼容旧接口，等同 totalTimeouts） */
    quint64 timeoutCount() const;

    /** @brief 获取Modbus异常响应计数（兼容旧接口，等同 totalErrors） */
    quint64 errorCount() const;

    /** @brief 重置统计数据（兼容旧接口，等同 resetStats） */
    void resetStatistics();

signals:
    /** @brief 收到有效响应 */
    void responseReceived(const ModbusFrame& frame);

    /** @brief 响应超时 */
    void timeout(int slave, int function);

    /** @brief Modbus异常响应 */
    void error(ModbusError errorCode);

private slots:
    /** @brief 处理底层连接收到的数据 */
    void onRawDataReceived(const QByteArray& data);

    /** @brief 响应超时处理 */
    void onTimeout();

private:
    /** @brief 发送原始帧数据 */
    bool sendFrame(const QByteArray& rawData);

    /** @brief 解析响应帧 */
    void parseResponse(const QByteArray& data);

    IConnection* m_connection = nullptr;  ///< 底层连接接口
    int          m_timeoutMs   = 1000;    ///< 超时时间（毫秒）
    quint8       m_lastSlave   = 1;       ///< 上次请求的从站地址
    QTimer*      m_timer       = nullptr; ///< 响应超时定时器
    QByteArray   m_rxBuffer;              ///< 接收缓冲区

    quint64 m_totalRequests  = 0;         ///< 累计发送请求总数
    quint64 m_totalResponses = 0;         ///< 累计接收有效响应总数
    quint64 m_totalTimeouts  = 0;         ///< 累计超时次数
    quint64 m_totalErrors    = 0;         ///< 累计Modbus异常响应总数
};

#endif // MODBUS_MASTER_H
