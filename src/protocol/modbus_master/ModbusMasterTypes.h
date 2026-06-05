/**
 * @file ModbusMasterTypes.h
 * @brief Modbus Master轮询层类型定义 -- 请求/响应结构体与功能码枚举
 *
 * 纯C++类型，无Q_OBJECT。用于ModbusMaster轮询调度器的请求队列与响应回调。
 * 底层帧编解码复用 protocol/modbus/ModbusTypes.h 中的 CRC16 和帧工具。
 */
#ifndef MODBUS_MASTER_TYPES_H
#define MODBUS_MASTER_TYPES_H

#include <QVector>
#include <QString>
#include <QtGlobal>

/** @brief Modbus RTU功能码枚举(轮询层子集) */
enum class ModbusMasterFunction : quint8 {
    ReadCoils            = 1,  ///< FC01 读线圈
    ReadDiscreteInputs   = 2,  ///< FC02 读离散输入
    ReadHoldingRegisters = 3,  ///< FC03 读保持寄存器
    ReadInputRegisters   = 4,  ///< FC04 读输入寄存器
    WriteSingleCoil      = 5,  ///< FC05 写单个线圈
    WriteSingleRegister  = 6,  ///< FC06 写单个寄存器
    WriteMultipleRegisters = 16 ///< FC16 写多个寄存器
};

/** @brief Modbus轮询请求结构体 */
struct ModbusMasterRequest {
    quint8              slaveAddr = 1;     ///< 从站地址 (1-247)
    ModbusMasterFunction func = ModbusMasterFunction::ReadHoldingRegisters; ///< 功能码
    quint16             startReg  = 0;     ///< 起始寄存器地址
    quint16             count     = 1;     ///< 寄存器数量/线圈数量
    QVector<quint16>    values;            ///< 写入值列表(写操作时填充)
};

/** @brief Modbus轮询响应结构体 */
struct ModbusMasterResponse {
    bool                success    = false; ///< 请求是否成功
    quint8              slaveAddr  = 0;     ///< 响应从站地址
    quint16             startReg   = 0;     ///< 响应起始寄存器
    ModbusMasterFunction func = ModbusMasterFunction::ReadHoldingRegisters; ///< 功能码
    QVector<quint16>    values;             ///< 读取到的寄存器值列表
    QString             error;              ///< 错误描述(失败时填充)
};

#endif // MODBUS_MASTER_TYPES_H
