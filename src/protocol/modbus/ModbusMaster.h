/** @file ModbusMaster.h @brief Modbus主站(客户端) -- 发起请求并处理响应。支持RTU/ASCII/TCP模式，提供标准功能码便捷方法和自定义帧发送。统计增强: 按功能码分类计数 */
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include "protocol/modbus/ModbusTypes.h"
#include "connection/interface/IConnection.h"

/** @brief Modbus主站控制器。负责构造请求帧、发送并等待响应、超时管理。提供运行时统计接口，按功能码追踪读写成功率 */
class ModbusMaster : public QObject {
    Q_OBJECT

public:
    explicit ModbusMaster(QObject* parent = nullptr);

    /** @brief 设置底层连接 */
    void setConnection(IConnection* connection);

    /** @brief 设置响应超时时间（毫秒） */
    void setTimeout(int ms);

    /** @brief 获取当前响应超时时间（毫秒） */
    int timeout() const;

    bool readCoils(int slave, int start, int count); ///< 读线圈状态(FC01)
    bool readHoldingRegisters(int slave, int start, int count); ///< 读保持寄存器(FC03)
    bool readInputRegisters(int slave, int start, int count); ///< 读输入寄存器(FC04)
    bool writeSingleRegister(int slave, int addr, quint16 value); ///< 写单个寄存器(FC06)
    bool writeMultipleRegisters(int slave, int addr, const QList<quint16>& values); ///< 写多个寄存器(FC16)
    bool sendCustomFrame(const ModbusFrame& frame); ///< 发送自定义Modbus帧

    // ---- 统计接口 ----
    quint64 totalRequests() const;            ///< 累计发送请求总数
    quint64 totalResponses() const;           ///< 累计接收有效响应总数
    quint64 totalTimeouts() const;            ///< 累计超时次数
    quint64 totalErrors() const;              ///< 累计Modbus异常响应总数
    quint64 successfulReads() const;          ///< 累计读操作成功次数
    quint64 failedReads() const;              ///< 累计读操作失败次数(含超时+异常)
    quint64 successfulWrites() const;         ///< 累计写操作成功次数
    quint64 failedWrites() const;             ///< 累计写操作失败次数(含超时+异常)
    quint64 functionCodeCount(int fc) const;  ///< 指定功能码的调用次数
    quint64 totalExceptions() const;          ///< 累计Modbus异常响应次数(功能码最高位置1)
    quint64 totalRetries() const;             ///< 累计重试发送次数
    void resetStats();                        ///< 重置所有统计计数器

    // ---- 兼容旧接口 ----
    bool readRegisters(int slave, int start, int count); ///< 读寄存器(默认FC03, 兼容旧接口)
    quint64 requestCount() const;             ///< 已发送请求计数(兼容旧接口, 等同totalRequests)
    quint64 responseCount() const;            ///< 已接收响应计数(兼容旧接口, 等同totalResponses)
    quint64 timeoutCount() const;             ///< 超时次数(兼容旧接口, 等同totalTimeouts)
    quint64 errorCount() const;               ///< 异常响应计数(兼容旧接口, 等同totalErrors)
    void resetStatistics();                   ///< 重置统计数据(兼容旧接口, 等同resetStats)

signals:
    void responseReceived(const ModbusFrame& frame); ///< 收到有效响应
    void timeout(int slave, int function);    ///< 响应超时
    void error(ModbusError errorCode);        ///< Modbus异常响应

private slots:
    void onRawDataReceived(const QByteArray& data); ///< 处理底层连接收到的数据
    void onTimeout();                         ///< 响应超时处理

private:
    bool sendFrame(const QByteArray& rawData); ///< 发送原始帧数据
    void parseResponse(const QByteArray& data); ///< 解析响应帧
    static bool isReadFunction(quint8 fc);     ///< 判断功能码是否为读操作
    static bool isWriteFunction(quint8 fc);    ///< 判断功能码是否为写操作

    IConnection* m_connection = nullptr;  ///< 底层连接接口
    int          m_timeoutMs   = 1000;    ///< 超时时间（毫秒）
    quint8       m_lastSlave   = 1;       ///< 上次请求的从站地址
    quint8       m_lastFunction = 3;      ///< 上次请求的功能码
    QTimer*      m_timer       = nullptr; ///< 响应超时定时器
    QByteArray   m_rxBuffer;              ///< 接收缓冲区

    quint64 m_totalRequests    = 0;       ///< 累计发送请求总数
    quint64 m_totalResponses   = 0;       ///< 累计接收有效响应总数
    quint64 m_totalTimeouts    = 0;       ///< 累计超时次数
    quint64 m_totalErrors      = 0;       ///< 累计Modbus异常响应总数
    quint64 m_totalExceptions  = 0;       ///< 累计Modbus异常响应次数（功能码最高位置1）
    quint64 m_totalRetries     = 0;       ///< 累计重试发送次数
    quint64 m_successfulReads  = 0;       ///< 累计读操作成功次数
    quint64 m_failedReads      = 0;       ///< 累计读操作失败次数
    quint64 m_successfulWrites = 0;       ///< 累计写操作成功次数
    quint64 m_failedWrites     = 0;       ///< 累计写操作失败次数
    QMap<int, quint64> m_fcStats;         ///< 各功能码调用次数
};

#endif // MODBUS_MASTER_H
