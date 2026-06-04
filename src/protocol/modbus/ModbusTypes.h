/** @file ModbusTypes.h @brief Modbus协议基础类型定义 -- 纯C++结构体，无Q_OBJECT。定义功能码、异常码和帧结构，提供CRC16/帧序列化/反序列化静态内联工具 */
#ifndef MODBUS_TYPES_H
#define MODBUS_TYPES_H

#include <QByteArray>
#include <QtGlobal>

/** @brief Modbus功能码枚举，对应协议标准定义 */
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

/** @brief Modbus异常码枚举，对应协议标准定义 */
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

/** @brief Modbus数据帧结构体，封装一次请求或响应的所有字段 */
struct ModbusFrame {
    quint8          slaveAddress  = 0;       ///< 从站地址 (1-247)
    ModbusFunction  function      = ModbusFunction::ReadHoldingRegisters; ///< 功能码
    quint16         startAddress  = 0;       ///< 寄存器/线圈起始地址
    quint16         quantity      = 0;       ///< 数量（寄存器/线圈个数）
    QByteArray      data;                    ///< 数据负载（不含CRC）
    bool            exception     = false;   ///< 是否为异常响应帧
};

/** @brief 计算Modbus CRC16校验值(多项式0xA001) @param data 待计算数据 @return CRC16校验值，低字节在前 */
inline quint16 crc16(const QByteArray& data) {
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x0001) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
    }
    return crc;
}

/** @brief 将Modbus帧序列化为完整RTU字节流(含CRC) @param frame 待序列化帧 @return 字节数组(含2字节CRC尾) */
inline QByteArray frameToBytes(const ModbusFrame& frame) {
    QByteArray bytes;
    bytes.append(static_cast<char>(frame.slaveAddress));
    bytes.append(static_cast<char>(frame.function));
    if (frame.exception) {
        bytes.append(frame.data.isEmpty() ? QByteArray(1, '\0') : frame.data);
    } else if (frame.function == ModbusFunction::WriteSingleCoil ||
               frame.function == ModbusFunction::WriteSingleRegister) {
        // FC05/06: 从站 + 功能码 + 起始地址(2) + 数据值
        bytes.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.startAddress & 0xFF));
        bytes.append(frame.data);
    } else if (frame.function == ModbusFunction::WriteMultipleCoils ||
               frame.function == ModbusFunction::WriteMultipleRegisters) {
        // FC15/16: 从站 + 功能码 + 起始地址(2) + 数量(2) + 字节计数 + 数据
        bytes.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.startAddress & 0xFF));
        bytes.append(static_cast<char>((frame.quantity >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.quantity & 0xFF));
        bytes.append(frame.data);
    } else {
        // FC01-04: 从站 + 功能码 + 起始地址(2) + 数量(2)
        bytes.append(static_cast<char>((frame.startAddress >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.startAddress & 0xFF));
        bytes.append(static_cast<char>((frame.quantity >> 8) & 0xFF));
        bytes.append(static_cast<char>(frame.quantity & 0xFF));
    }
    quint16 crcVal = crc16(bytes);
    bytes.append(static_cast<char>(crcVal & 0xFF));
    bytes.append(static_cast<char>((crcVal >> 8) & 0xFF));
    return bytes;
}

/** @brief 从字节流解析Modbus RTU帧(校验CRC) @param bytes 待解析字节数组(含CRC尾) @return 解析帧，CRC错误时字段可能无效 */
inline ModbusFrame bytesToFrame(const QByteArray& bytes) {
    ModbusFrame frame;
    if (bytes.size() < 4) return frame; // 最小帧: slave(1) + func(1) + CRC(2)
    frame.slaveAddress = static_cast<quint8>(bytes[0]);
    quint8 funcCode    = static_cast<quint8>(bytes[1]);
    QByteArray payload = bytes.left(bytes.size() - 2);
    // 异常响应判断（功能码最高位为1）
    if (funcCode & 0x80) {
        frame.exception = true;
        frame.function  = static_cast<ModbusFunction>(funcCode & 0x7F);
        if (payload.size() >= 3) frame.data = payload.mid(2);
        return frame;
    }
    frame.function = static_cast<ModbusFunction>(funcCode);
    bool isReadFunc = (funcCode >= 0x01 && funcCode <= 0x04);
    if (isReadFunc) {
        if (payload.size() == 6) {
            // 请求帧: slave + func + 起始地址(2) + 数量(2)
            frame.startAddress = (static_cast<quint8>(payload[2]) << 8) | static_cast<quint8>(payload[3]);
            frame.quantity     = (static_cast<quint8>(payload[4]) << 8) | static_cast<quint8>(payload[5]);
        } else if (payload.size() >= 3) {
            // 响应帧: slave + func + 字节计数(1) + 数据(N)
            quint8 byteCount = static_cast<quint8>(payload[2]);
            if (byteCount == payload.size() - 3) frame.data = payload.mid(3, byteCount);
        }
    } else if (funcCode == 0x05 || funcCode == 0x06) {
        if (payload.size() >= 6) {
            frame.startAddress = (static_cast<quint8>(payload[2]) << 8) | static_cast<quint8>(payload[3]);
            frame.data = payload.mid(4);
        }
    } else if (funcCode == 0x0F || funcCode == 0x10) {
        if (payload.size() >= 6) {
            frame.startAddress = (static_cast<quint8>(payload[2]) << 8) | static_cast<quint8>(payload[3]);
            frame.quantity     = (static_cast<quint8>(payload[4]) << 8) | static_cast<quint8>(payload[5]);
            if (payload.size() > 6) frame.data = payload.mid(6);
        }
    }
    return frame;
}

#endif // MODBUS_TYPES_H
