/**
 * @file ModbusSlaveResponse.cpp
 * @brief Modbus从站 — 响应构建方法
 *
 * 从 ModbusSlave.cpp 中拆分出的响应构造功能，职责:
 *   1. buildReadRegistersResponse        — FC03/FC04 读寄存器响应
 *   2. buildReadCoilsResponse            — FC01/FC02 读线圈响应
 *   3. buildWriteSingleCoilResponse      — FC05 写单线圈响应
 *   4. buildWriteSingleRegisterResponse  — FC06 写单寄存器响应
 *   5. buildWriteMultipleCoilsResponse   — FC15 写多线圈响应
 *   6. buildWriteMultipleRegistersResponse — FC16 写多寄存器响应
 *   7. buildExceptionResponse            — 异常响应构造
 *   8. calculateCrc16                    — CRC16校验委托
 *
 * 统计查询方法已拆分至 ModbusSlaveStats.cpp
 */

#include "protocol/modbus/ModbusSlave.h"

// ============================================================================
// 响应构建方法
// ============================================================================

/** @brief 构建FC03/FC04读寄存器响应(大端序数据) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildReadRegistersResponse(const ModbusFrame& req) {
    /* 防御: quantity上限检查(Modbus最大125个寄存器) */
    if (req.quantity < 1 || req.quantity > 125) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));

    // 字节数 = 寄存器数量 × 2
    int byteCount = req.quantity * 2;
    response.append(static_cast<char>(byteCount));

    // 逐个寄存器追加数据（大端序）
    for (int i = 0; i < req.quantity; ++i) {
        quint16 val = registerValue(req.startAddress + i);
        response.append(static_cast<char>((val >> 8) & 0xFF));
        response.append(static_cast<char>(val & 0xFF));
    }
    return response;
}

/** @brief 构建FC01/FC02读线圈响应(每8线圈打包为1字节) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildReadCoilsResponse(const ModbusFrame& req) {
    /* 防御: quantity上限检查(Modbus最大2000个线圈) */
    if (req.quantity < 1 || req.quantity > 2000) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));

    // 计算需要的字节数
    int byteCount = (req.quantity + 7) / 8;
    response.append(static_cast<char>(byteCount));

    // 每8个线圈打包为1字节
    for (int byteIdx = 0; byteIdx < byteCount; ++byteIdx) {
        quint8 byteVal = 0;
        for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
            int addr = req.startAddress + byteIdx * 8 + bitIdx;
            if (addr < req.startAddress + req.quantity && coilValue(addr)) {
                byteVal |= (1 << bitIdx);
            }
        }
        response.append(static_cast<char>(byteVal));
    }
    return response;
}

/** @brief 构建FC05写单线圈响应(写入+回显请求) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildWriteSingleCoilResponse(const ModbusFrame& req) {
    // FC05: 写入线圈值并回显请求
    if (req.data.size() >= 2) {
        quint16 value = (static_cast<quint8>(req.data[0]) << 8) |
                         static_cast<quint8>(req.data[1]);
        setCoilValue(req.startAddress, (value == 0xFF00));
    }

    // 回显: slave + func + 输出地址(2) + 输出值(2)
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));
    response.append(static_cast<char>((req.startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(req.startAddress & 0xFF));
    response.append(req.data);
    return response;
}

/** @brief 构建FC06写单寄存器响应(写入+回显请求) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildWriteSingleRegisterResponse(const ModbusFrame& req) {
    // FC06: 写入寄存器值并回显请求
    if (req.data.size() >= 2) {
        quint16 value = (static_cast<quint8>(req.data[0]) << 8) |
                         static_cast<quint8>(req.data[1]);
        setRegisterValue(req.startAddress, value);
    }

    // 回显: slave + func + 寄存器地址(2) + 寄存器值(2)
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));
    response.append(static_cast<char>((req.startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(req.startAddress & 0xFF));
    response.append(req.data);
    return response;
}

/** @brief 构建FC15写多线圈响应(解析线圈数据+写入+回显起始地址和数量) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildWriteMultipleCoilsResponse(const ModbusFrame& req) {
    // FC15: 解析线圈数据并写入
    if (req.data.size() < 1) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }

    // req.data = [byteCount, coilData...]
    for (int i = 0; i < req.quantity; ++i) {
        int byteIdx = 1 + (i / 8);
        int bitIdx = i % 8;
        if (byteIdx < req.data.size()) {
            bool value = (static_cast<quint8>(req.data[byteIdx]) >> bitIdx) & 0x01;
            setCoilValue(req.startAddress + i, value);
        }
    }

    // 响应: slave + func + 起始地址(2) + 数量(2)
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));
    response.append(static_cast<char>((req.startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(req.startAddress & 0xFF));
    response.append(static_cast<char>((req.quantity >> 8) & 0xFF));
    response.append(static_cast<char>(req.quantity & 0xFF));
    return response;
}

/** @brief 构建FC16写多寄存器响应(解析寄存器数据+写入+回显起始地址和数量) @param req 请求帧 @return 响应帧(不含CRC) */
QByteArray ModbusSlave::buildWriteMultipleRegistersResponse(const ModbusFrame& req) {
    // FC16: 解析寄存器数据并写入
    if (req.data.size() < 1) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }

    // req.data = [byteCount, regData...]
    quint8 byteCount = static_cast<quint8>(req.data[0]);
    /* 防御: byteCount必须与quantity×2一致 */
    if (byteCount != req.quantity * 2) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }
    if (req.data.size() < 1 + byteCount) {
        return buildExceptionResponse(req, ModbusError::IllegalValue);
    }

    for (int i = 0; i < req.quantity; ++i) {
        int offset = 1 + i * 2;
        if (offset + 1 >= req.data.size()) { break; }
        quint16 value = (static_cast<quint8>(req.data[offset]) << 8) |
                         static_cast<quint8>(req.data[offset + 1]);
        setRegisterValue(req.startAddress + i, value);
    }

    // 响应: slave + func + 起始地址(2) + 数量(2)
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));
    response.append(static_cast<char>((req.startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(req.startAddress & 0xFF));
    response.append(static_cast<char>((req.quantity >> 8) & 0xFF));
    response.append(static_cast<char>(req.quantity & 0xFF));
    return response;
}

/** @brief 构建Modbus异常响应(功能码最高位置1+异常码) @param req 请求帧 @param err 异常码 @return 异常响应帧(不含CRC) */
QByteArray ModbusSlave::buildExceptionResponse(const ModbusFrame& req,
                                                ModbusError err) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(
        static_cast<quint8>(req.function) | 0x80));
    response.append(static_cast<char>(err));
    return response;
}

/** @brief 计算CRC16校验值(委托全局crc16函数) @param data 待校验数据 @return CRC16校验值 */
quint16 ModbusSlave::calculateCrc16(const QByteArray& data) const {
    return crc16(data);
}

// ---- 统计查询方法见 ModbusSlaveStats.cpp ----
