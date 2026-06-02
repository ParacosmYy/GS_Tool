/**
 * @file ModbusTypes.h
 * @brief Modbus协议基础类型定义 — 纯C++结构体，无Q_OBJECT
 *
 * 定义Modbus RTU/ASCII/TCP协议所需的功能码、异常码和帧结构。
 * 提供帧序列化/反序列化的静态内联工具函数。
 */
#ifndef MODBUS_TYPES_H
#define MODBUS_TYPES_H

#include <QByteArray>
#include <QtGlobal>

/**
 * @brief Modbus功能码枚举
 * 对应Modbus协议标准定义的功能码
 */
enum class ModbusFunction : quint8 {
    ReadCoils              = 1,   ///< 读线圈 (FC01)
    ReadDiscreteInputs     = 2,   ///< 读离散输入 (FC02)
    ReadHoldingRegisters   = 3,   ///< 读保持寄存器 (FC03)
    ReadInputRegisters     = 4,   ///< 读输入寄存器 (FC04)
    WriteSingleCoil        = 5,   ///< 写单个线圈 (FC05)
    WriteSingleRegister    = 6,   ///< 写单个寄存器 (FC06)
    WriteMultipleCoils     = 15,  ///< 写多个线圈 (FC15)
    WriteMultipleRegisters = 16   ///< 写多个寄存器 (FC16)
};

/**
 * @brief Modbus异常码枚举
 * 对应Modbus协议标准定义的异常响应码
 */
enum class ModbusError : quint8 {
    IllegalFunction = 1,  ///< 非法功能码
    IllegalAddress  = 2,  ///< 非法数据地址
    IllegalValue    = 3,  ///< 非法数据值
    ServerFailure   = 4,  ///< 服务器设备故障
    Acknowledge     = 5,  ///< 确认（长时间操作）
    ServerBusy      = 6,  ///< 服务器设备忙
    GatewayPath     = 10, ///< 网关路径不可用
    GatewayTarget   = 11  ///< 网关目标设备无响应
};

/**
 * @brief Modbus数据帧结构体
 * 封装一次Modbus请求或响应的所有字段
 */
struct ModbusFrame {
    quint8          slaveAddress  = 0;       ///< 从站地址 (1-247)
    ModbusFunction  function      = ModbusFunction::ReadHoldingRegisters; ///< 功能码
    quint16         startAddress  = 0;       ///< 寄存器/线圈起始地址
    quint16         quantity      = 0;       ///< 数量（寄存器/线圈个数）
    QByteArray      data;                    ///< 数据负载（不含CRC）
    bool            exception     = false;   ///< 是否为异常响应帧
};

/**
 * @brief 将Modbus帧序列化为字节流（不含CRC）
 * @param frame 待序列化的Modbus帧
 * @return 序列化后的字节数组
 */
inline QByteArray frameToBytes(const ModbusFrame& frame) {
    QByteArray bytes;
    bytes.append(static_cast<char>(frame.slaveAddress));
    bytes.append(static_cast<char>(frame.function));

    if (frame.exception) {
        bytes.append(frame.data.isEmpty() ? QByteArray(1, '\0') : frame.data);
        return bytes;
    }

    const bool isWrite = (frame.function == ModbusFunction::WriteSingleCoil ||
                          frame.function == ModbusFunction::WriteSingleRegister);
    if (isWrite) {
        // 写操作: 起始地址 + 数据值
        bytes.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.startAddress & 0xFF));
        bytes.append(frame.data);
    } else {
        // 读操作: 起始地址 + 数量
        bytes.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.startAddress & 0xFF));
        bytes.append(static_cast<char>((frame.quantity >> 8) & 0xFF));
        bytes.append(static_cast<char>((frame.quantity & 0xFF)));
    }
    return bytes;
}

/**
 * @brief 从字节流解析Modbus帧（不含CRC）
 * @param bytes 待解析的字节数组
 * @return 解析得到的Modbus帧
 */
inline ModbusFrame bytesToFrame(const QByteArray& bytes) {
    ModbusFrame frame;
    if (bytes.size() < 2) { return frame; }

    frame.slaveAddress = static_cast<quint8>(bytes[0]);
    quint8 funcCode    = static_cast<quint8>(bytes[1]);

    // 判断是否为异常响应 (功能码最高位为1)
    if (funcCode & 0x80) {
        frame.exception = true;
        frame.function  = static_cast<ModbusFunction>(funcCode & 0x7F);
    } else {
        frame.function = static_cast<ModbusFunction>(funcCode);
    }

    if (bytes.size() >= 5) {
        frame.startAddress = (static_cast<quint8>(bytes[2]) << 8) |
                              static_cast<quint8>(bytes[3]);
        frame.quantity = (static_cast<quint8>(bytes[4]) << 8) |
                          static_cast<quint8>(bytes[5] ? bytes[5] : 0);
        if (bytes.size() > 5) {
            frame.data = bytes.mid(5);
        }
    }
    return frame;
}

#endif // MODBUS_TYPES_H
