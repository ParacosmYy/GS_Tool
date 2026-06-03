/**
 * @file ModbusMaster.h
 * @brief Modbus主站（客户端）— 发起请求并处理响应
 *
 * 通过IConnection接口发送Modbus请求帧，接收并解析响应。
 * 支持RTU/ASCII/TCP模式，提供标准功能码便捷方法和自定义帧发送。
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

    /** @brief 获取已发送请求计数 */
    quint64 requestCount() const;

    /** @brief 获取已接收响应计数 */
    quint64 responseCount() const;

    /** @brief 获取超时次数 */
    quint64 timeoutCount() const;

    /** @brief 获取Modbus异常响应计数 */
    quint64 errorCount() const;

    /** @brief 重置统计数据 */
    void resetStatistics();

    /** @brief 重置统计数据(等同于resetStatistics) */
    void resetStats();

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

    /** @brief 已发送请求计数 */
    quint64 m_requestCount = 0;
    /** @brief 已接收响应计数 */
    quint64 m_responseCount = 0;
    /** @brief 超时次数 */
    quint64 m_timeoutCount = 0;
    /** @brief Modbus异常响应计数 */
    quint64 m_errorCount = 0;
};

#endif // MODBUS_MASTER_H
