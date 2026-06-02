/**
 * @file ModbusSlave.cpp
 * @brief Modbus从站（模拟器）实现
 *
 * 维护寄存器和线圈的内存映射，解析请求帧并构造响应。
 * 支持FC01-06, FC15-16，CRC16校验完整。
 */
#include "protocol/modbus/ModbusSlave.h"

ModbusSlave::ModbusSlave(QObject* parent)
    : QObject(parent)
{
}

void ModbusSlave::setSlaveAddress(int address) {
    m_slaveAddress = static_cast<quint8>(address);
}

quint8 ModbusSlave::slaveAddress() const {
    return m_slaveAddress;
}

void ModbusSlave::setRegisterValue(int addr, quint16 value) {
    m_registers[addr] = value;
}

quint16 ModbusSlave::registerValue(int addr) const {
    return m_registers.value(addr, 0);
}

void ModbusSlave::setCoilValue(int addr, bool value) {
    m_coils[addr] = value;
}

bool ModbusSlave::coilValue(int addr) const {
    return m_coils.value(addr, false);
}

QByteArray ModbusSlave::processRequest(const QByteArray& requestData) {
    // 最小帧长度: slave(1)+func(1)+CRC(2) = 4字节
    if (requestData.size() < 4) {
        return QByteArray();
    }

    // CRC16校验
    QByteArray payload = requestData.left(requestData.size() - 2);
    quint16 recvCrc = static_cast<quint8>(requestData[requestData.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(
                          requestData[requestData.size() - 1])) << 8);
    if (calculateCrc16(payload) != recvCrc) {
        return QByteArray(); // CRC校验失败
    }

    ModbusFrame req = bytesToFrame(requestData);

    // 检查从站地址是否匹配
    if (req.slaveAddress != m_slaveAddress) {
        return QByteArray();
    }

    // 根据功能码分派处理
    QByteArray responsePayload;
    switch (req.function) {
    case ModbusFunction::ReadHoldingRegisters:
    case ModbusFunction::ReadInputRegisters:
        responsePayload = buildReadRegistersResponse(req);
        break;
    case ModbusFunction::ReadCoils:
    case ModbusFunction::ReadDiscreteInputs:
        responsePayload = buildReadCoilsResponse(req);
        break;
    case ModbusFunction::WriteSingleCoil:
        responsePayload = buildWriteSingleCoilResponse(req);
        break;
    case ModbusFunction::WriteSingleRegister:
        responsePayload = buildWriteSingleRegisterResponse(req);
        break;
    case ModbusFunction::WriteMultipleCoils:
        responsePayload = buildWriteMultipleCoilsResponse(req);
        break;
    case ModbusFunction::WriteMultipleRegisters:
        responsePayload = buildWriteMultipleRegistersResponse(req);
        break;
    default:
        responsePayload = buildExceptionResponse(
            req, ModbusError::IllegalFunction);
        break;
    }

    if (responsePayload.isEmpty()) {
        return QByteArray();
    }

    // 追加CRC16校验（小端序: 低字节在前）
    quint16 crcVal = calculateCrc16(responsePayload);
    responsePayload.append(static_cast<char>(crcVal & 0xFF));
    responsePayload.append(static_cast<char>((crcVal >> 8) & 0xFF));
    return responsePayload;
}

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

QByteArray ModbusSlave::buildExceptionResponse(const ModbusFrame& req,
                                                ModbusError err) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(
        static_cast<quint8>(req.function) | 0x80));
    response.append(static_cast<char>(err));
    return response;
}

quint16 ModbusSlave::calculateCrc16(const QByteArray& data) const {
    return crc16(data);
}
