/**
 * @file ModbusSlave.h
 * @brief Modbus从站（模拟器）— 模拟从站设备响应主站请求
 *
 * 维护寄存器和线圈的内存映射，接收请求帧并返回响应。
 * 支持FC01-06, FC15-16功能码，用于测试Modbus主站功能。
 */
#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <QObject>
#include <QMap>
#include "protocol/modbus/ModbusTypes.h"

/**
 * @brief Modbus从站模拟器
 * 模拟从站设备，维护寄存器/线圈数据并响应请求。
 */
class ModbusSlave : public QObject {
    Q_OBJECT

public:
    /** @brief 构造Modbus从站模拟器 @param parent 父对象 */
    explicit ModbusSlave(QObject* parent = nullptr);

    /** @brief 设置从站地址 @param address 从站地址(1-247) */
    void setSlaveAddress(int address);

    /** @brief 获取当前从站地址 @return 从站地址 */
    quint8 slaveAddress() const;

    /**
     * @brief 设置保持寄存器值
     * @param addr 寄存器地址
     * @param value 寄存器值
     */
    void setRegisterValue(int addr, quint16 value);

    /**
     * @brief 读取保持寄存器值
     * @param addr 寄存器地址
     * @return 寄存器当前值
     */
    quint16 registerValue(int addr) const;

    /**
     * @brief 设置线圈状态
     * @param addr 线圈地址
     * @param value 线圈状态（true=ON, false=OFF）
     */
    void setCoilValue(int addr, bool value);

    /**
     * @brief 读取线圈状态
     * @param addr 线圈地址
     * @return 线圈当前状态
     */
    bool coilValue(int addr) const;

    /**
     * @brief 处理收到的Modbus请求并返回响应
     * @param requestData 原始请求帧数据（含CRC）
     * @return 响应帧数据（含CRC），异常时返回异常响应
     */
    QByteArray processRequest(const QByteArray& requestData);

    /** @brief 获取已处理请求总数 */
    quint64 requestCount() const;

    /** @brief 获取异常响应计数 */
    quint64 exceptionCount() const;

    /** @brief 获取已成功处理的请求总数（地址匹配+CRC通过） @return 已处理请求数 */
    quint64 totalRequestsHandled() const;

    /** @brief 获取已发送的响应帧总数（含正常+异常） @return 响应发送总数 */
    quint64 totalResponsesSent() const;

    /** @brief 获取从站内部错误次数（CRC失败/帧过短等） @return 内部错误计数 */
    quint64 totalSlaveErrors() const;

    /** @brief 获取异常响应发送总数（功能码最高位置1的响应） @return 异常响应计数 */
    quint64 totalExceptionResponses() const;

    /** @brief 获取各功能码调用次数统计 */
    QMap<int, int> functionCodeStats() const;

    /** @brief 获取CRC校验失败次数 @return CRC错误计数 */
    quint64 totalCrcErrors() const;

    /** @brief 获取不支持功能码被调用的次数 @return 不支持功能码计数 */
    quint64 totalUnsupportedFunctions() const;

    /** @brief 重置统计数据 */
    void resetStatistics();

private:
    /** @brief 构造读寄存器响应 */
    QByteArray buildReadRegistersResponse(const ModbusFrame& req);

    /** @brief 构造读线圈响应 */
    QByteArray buildReadCoilsResponse(const ModbusFrame& req);

    /** @brief 构造写单个线圈响应 (FC05) */
    QByteArray buildWriteSingleCoilResponse(const ModbusFrame& req);

    /** @brief 构造写单个寄存器响应 */
    QByteArray buildWriteSingleRegisterResponse(const ModbusFrame& req);

    /** @brief 构造写多个线圈响应 (FC15) */
    QByteArray buildWriteMultipleCoilsResponse(const ModbusFrame& req);

    /** @brief 构造写多个寄存器响应 (FC16) */
    QByteArray buildWriteMultipleRegistersResponse(const ModbusFrame& req);

    /** @brief 构造异常响应 */
    QByteArray buildExceptionResponse(const ModbusFrame& req, ModbusError err);

    /** @brief 计算CRC16校验 */
    quint16 calculateCrc16(const QByteArray& data) const;

    QMap<int, quint16> m_registers;        ///< 保持寄存器映射 (地址→值)
    QMap<int, bool>    m_coils;             ///< 线圈映射 (地址→状态)
    quint8             m_slaveAddress = 1;  ///< 从站地址

    quint64 m_requestCount = 0;             ///< 已处理请求计数
    quint64 m_exceptionCount = 0;           ///< 异常响应计数
    quint64 m_totalRequestsHandled = 0;     ///< 已成功处理的请求总数（地址匹配+CRC通过）
    quint64 m_totalResponsesSent = 0;       ///< 已发送的响应帧总数（含正常+异常）
    quint64 m_totalSlaveErrors = 0;         ///< 从站内部错误次数（CRC失败/帧过短等）
    quint64 m_totalExceptionResponses = 0;  ///< 异常响应发送总数（功能码最高位置1的响应）
    quint64 m_totalCrcErrors = 0;           ///< CRC校验失败次数
    quint64 m_totalUnsupportedFunctions = 0; ///< 不支持功能码被调用的次数
    QMap<int, int> m_fcStats;               ///< 各功能码调用次数
};

#endif // MODBUS_SLAVE_H
