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
    /** @brief 构造Modbus主站控制器 @param parent 父对象 */
    explicit ModbusMaster(QObject* parent = nullptr);

    /** @brief 设置底层连接 @param connection IConnection连接实例 */
    void setConnection(IConnection* connection);

    /** @brief 设置响应超时时间 @param ms 超时毫秒数 */
    void setTimeout(int ms);

    /** @brief 获取当前响应超时时间 @return 超时毫秒数 */
    int timeout() const;

    /** @brief 读线圈状态(FC01) @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=请求已发送 */
    bool readCoils(int slave, int start, int count);
    /** @brief 读保持寄存器(FC03) @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=请求已发送 */
    bool readHoldingRegisters(int slave, int start, int count);
    /** @brief 读输入寄存器(FC04) @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=请求已发送 */
    bool readInputRegisters(int slave, int start, int count);
    /** @brief 写单个寄存器(FC06) @param slave 从站地址 @param addr 寄存器地址 @param value 写入值 @return true=请求已发送 */
    bool writeSingleRegister(int slave, int addr, quint16 value);
    /** @brief 写多个寄存器(FC16) @param slave 从站地址 @param addr 起始地址 @param values 写入值列表 @return true=请求已发送 */
    bool writeMultipleRegisters(int slave, int addr, const QList<quint16>& values);
    /** @brief 发送自定义Modbus帧 @param frame Modbus帧结构 @return true=发送成功 */
    bool sendCustomFrame(const ModbusFrame& frame);

    // ---- 统计接口 ----
    quint64 totalRequests() const;            ///< @brief 累计发送请求总数 @return 请求次数
    quint64 totalResponses() const;           ///< @brief 累计接收有效响应总数 @return 响应次数
    quint64 totalTimeouts() const;            ///< @brief 累计超时次数 @return 超时次数
    quint64 totalErrors() const;              ///< @brief 累计Modbus异常响应总数 @return 异常次数
    quint64 successfulReads() const;          ///< @brief 累计读操作成功次数 @return 成功次数
    quint64 failedReads() const;              ///< @brief 累计读操作失败次数(含超时+异常) @return 失败次数
    quint64 successfulWrites() const;         ///< @brief 累计写操作成功次数 @return 成功次数
    quint64 failedWrites() const;             ///< @brief 累计写操作失败次数(含超时+异常) @return 失败次数
    /** @brief 获取指定功能码的调用次数 @param fc 功能码 @return 调用次数 */
    quint64 functionCodeCount(int fc) const;
    quint64 totalExceptions() const;          ///< @brief 累计Modbus异常响应次数(功能码最高位置1) @return 异常次数
    quint64 totalRetries() const;             ///< @brief 累计重试发送次数 @return 重试次数
    /** @brief 重置所有统计计数器 */
    void resetStats();

    // ---- 兼容旧接口 ----
    /** @brief 读寄存器(默认FC03, 兼容旧接口) @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=请求已发送 */
    bool readRegisters(int slave, int start, int count);
    quint64 requestCount() const;             ///< @brief 已发送请求计数(兼容旧接口) @return 请求次数
    quint64 responseCount() const;            ///< @brief 已接收响应计数(兼容旧接口) @return 响应次数
    quint64 timeoutCount() const;             ///< @brief 超时次数(兼容旧接口) @return 超时次数
    quint64 errorCount() const;               ///< @brief 异常响应计数(兼容旧接口) @return 异常次数
    /** @brief 重置统计数据(兼容旧接口, 等同resetStats) */
    void resetStatistics();

signals:
    /** @brief 收到有效响应 @param frame 响应帧数据 */
    void responseReceived(const ModbusFrame& frame);
    /** @brief 响应超时 @param slave 从站地址 @param function 功能码 */
    void timeout(int slave, int function);
    /** @brief Modbus异常响应 @param errorCode 异常码 */
    void error(ModbusError errorCode);

private slots:
    /** @brief 处理底层连接收到的原始数据 @param data 原始字节流 */
    void onRawDataReceived(const QByteArray& data);
    /** @brief 响应超时处理，累加超时计数并尝试恢复 */
    void onTimeout();

private:
    /** @brief 发送原始帧数据到底层连接 @param rawData 完整帧字节(含CRC) @return true=发送成功 */
    bool sendFrame(const QByteArray& rawData);
    /** @brief 解析底层连接收到的响应帧 @param data 原始响应数据 */
    void parseResponse(const QByteArray& data);
    /** @brief 判断功能码是否为读操作 @param fc 功能码 @return true=读操作 */
    static bool isReadFunction(quint8 fc);
    /** @brief 判断功能码是否为写操作 @param fc 功能码 @return true=写操作 */
    static bool isWriteFunction(quint8 fc);

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
