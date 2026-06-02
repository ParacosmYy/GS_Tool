/**
 * @file ModbusSlave.cpp
 * @brief Modbus从站（模拟器）实现
 *
 * 维护寄存器和线圈的内存映射，解析请求帧并构造响应。
 * TODO: 实现完整的CRC16校验和所有功能码处理。
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
    // TODO: 校验CRC16
    if (requestData.size() < 4) {
        return QByteArray();
    }

    ModbusFrame req = bytesToFrame(requestData);

    // 检查从站地址是否匹配
    if (req.slaveAddress != m_slaveAddress) {
        return QByteArray();
    }

    switch (req.function) {
    case ModbusFunction::ReadHoldingRegisters:
    case ModbusFunction::ReadInputRegisters:
        return buildReadRegistersResponse(req);
    case ModbusFunction::ReadCoils:
    case ModbusFunction::ReadDiscreteInputs:
        return buildReadCoilsResponse(req);
    case ModbusFunction::WriteSingleRegister:
        return buildWriteSingleRegisterResponse(req);
    case ModbusFunction::WriteMultipleRegisters:
        // TODO: 实现写多寄存器
        return buildExceptionResponse(req, ModbusError::IllegalFunction);
    default:
        return buildExceptionResponse(req, ModbusError::IllegalFunction);
    }
}

QByteArray ModbusSlave::buildReadRegistersResponse(const ModbusFrame& req) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));

    // 字节数
    int byteCount = req.quantity * 2;
    response.append(static_cast<char>(byteCount));

    // 寄存器数据
    for (int i = 0; i < req.quantity; ++i) {
        quint16 val = registerValue(req.startAddress + i);
        response.append(static_cast<char>((val >> 8) & 0xFF));
        response.append(static_cast<char>(val & 0xFF));
    }

    // TODO: 追加CRC16
    return response;
}

QByteArray ModbusSlave::buildReadCoilsResponse(const ModbusFrame& req) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));

    int byteCount = (req.quantity + 7) / 8;
    response.append(static_cast<char>(byteCount));

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

    // TODO: 追加CRC16
    return response;
}

QByteArray ModbusSlave::buildWriteSingleRegisterResponse(const ModbusFrame& req) {
    int addr = req.startAddress;
    if (req.data.size() >= 2) {
        quint16 value = (static_cast<quint8>(req.data[0]) << 8) |
                         static_cast<quint8>(req.data[1]);
        setRegisterValue(addr, value);
    }

    // 回显请求帧作为响应
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(req.function));
    response.append(static_cast<char>((req.startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(req.startAddress & 0xFF));
    response.append(req.data);
    // TODO: 追加CRC16
    return response;
}

QByteArray ModbusSlave::buildExceptionResponse(const ModbusFrame& req,
                                                ModbusError err) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveAddress));
    response.append(static_cast<char>(
        static_cast<quint8>(req.function) | 0x80));
    response.append(static_cast<char>(err));
    // TODO: 追加CRC16
    return response;
}

quint16 ModbusSlave::calculateCrc16(const QByteArray& data) const {
    // TODO: 实现Modbus CRC16
    Q_UNUSED(data)
    return 0;
}
