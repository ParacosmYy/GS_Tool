/**
 * @file ModbusSlave.cpp
 * @brief Modbus从站（模拟器）实现
 *
 * 维护寄存器和线圈的内存映射，解析请求帧并构造响应。
 * 支持FC01-06, FC15-16，CRC16校验完整。
 */
#include "protocol/modbus/ModbusSlave.h"

/** @brief 构造Modbus从站模拟器 @param parent 父对象 */
ModbusSlave::ModbusSlave(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置从站地址 @param address 从站地址(1-247) */
void ModbusSlave::setSlaveAddress(int address) {
    m_slaveAddress = static_cast<quint8>(address);
}

/** @brief 获取从站地址 @return 当前从站地址 */
quint8 ModbusSlave::slaveAddress() const {
    return m_slaveAddress;
}

/** @brief 设置保持寄存器值 @param addr 寄存器地址 @param value 寄存器值 */
void ModbusSlave::setRegisterValue(int addr, quint16 value) {
    m_registers[addr] = value;
}

/** @brief 获取保持寄存器值 @param addr 寄存器地址 @return 寄存器值，不存在时返回0 */
quint16 ModbusSlave::registerValue(int addr) const {
    return m_registers.value(addr, 0);
}

/** @brief 设置线圈值 @param addr 线圈地址 @param value 线圈状态 */
void ModbusSlave::setCoilValue(int addr, bool value) {
    m_coils[addr] = value;
}

/** @brief 获取线圈值 @param addr 线圈地址 @return 线圈状态，不存在时返回false */
bool ModbusSlave::coilValue(int addr) const {
    return m_coils.value(addr, false);
}

/** @brief 处理接收到的Modbus请求帧(CRC校验+地址匹配+功能码分派) @param requestData 原始请求帧数据 @return 响应帧数据(含CRC16)，无效请求返回空 */
QByteArray ModbusSlave::processRequest(const QByteArray& requestData) {
    // 最小帧长度: slave(1)+func(1)+CRC(2) = 4字节
    if (requestData.size() < 4) {
        ++m_totalSlaveErrors;
        return QByteArray();
    }

    ++m_requestCount;

    // CRC16校验
    QByteArray payload = requestData.left(requestData.size() - 2);
    quint16 recvCrc = static_cast<quint8>(requestData[requestData.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(
                          requestData[requestData.size() - 1])) << 8);
    if (calculateCrc16(payload) != recvCrc) {
        ++m_totalSlaveErrors;
        return QByteArray(); // CRC校验失败
    }

    ModbusFrame req = bytesToFrame(requestData);

    // 检查从站地址是否匹配
    if (req.slaveAddress != m_slaveAddress) {
        return QByteArray();
    }

    // 根据功能码分派处理
    QByteArray responsePayload;
    ++m_totalRequestsHandled;
    ++m_fcStats[static_cast<int>(req.function)];
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
        ++m_exceptionCount;
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
    ++m_totalResponsesSent;
    // 检测异常响应（功能码最高位置1）
    if (responsePayload.size() >= 2
        && (static_cast<quint8>(responsePayload[1]) & 0x80)) {
        ++m_totalExceptionResponses;
    }
    return responsePayload;
}

// ── 响应构建与统计查询方法已拆分至 ModbusSlaveResponse.cpp ──
// buildReadRegistersResponse / buildReadCoilsResponse / buildWriteSingleCoilResponse
// buildWriteSingleRegisterResponse / buildWriteMultipleCoilsResponse
// buildWriteMultipleRegistersResponse / buildExceptionResponse / calculateCrc16
// requestCount / exceptionCount / totalRequestsHandled / totalResponsesSent
// totalSlaveErrors / totalExceptionResponses / functionCodeStats / resetStatistics
