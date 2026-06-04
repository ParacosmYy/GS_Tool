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

/** @brief 获取已处理请求总数 @return 请求数 */
quint64 ModbusSlave::requestCount() const
{
    return m_requestCount;
}

/** @brief 获取异常响应计数 @return 异常数 */
quint64 ModbusSlave::exceptionCount() const
{
    return m_exceptionCount;
}

/** @brief 获取已成功处理的请求总数 @return 已处理请求数 */
quint64 ModbusSlave::totalRequestsHandled() const
{
    return m_totalRequestsHandled;
}

/** @brief 获取已发送的响应帧总数 @return 响应发送总数 */
quint64 ModbusSlave::totalResponsesSent() const
{
    return m_totalResponsesSent;
}

/** @brief 获取从站内部错误次数 @return 内部错误计数 */
quint64 ModbusSlave::totalSlaveErrors() const
{
    return m_totalSlaveErrors;
}

/** @brief 获取异常响应发送总数 @return 异常响应计数 */
quint64 ModbusSlave::totalExceptionResponses() const
{
    return m_totalExceptionResponses;
}

/** @brief 获取各功能码调用次数统计 @return 功能码→调用次数映射 */
QMap<int, int> ModbusSlave::functionCodeStats() const
{
    return m_fcStats;
}

/** @brief 重置所有统计计数器(请求数/异常数/功能码统计/已处理/已发送/内部错误/异常响应) */
void ModbusSlave::resetStatistics()
{
    m_requestCount = 0;
    m_exceptionCount = 0;
    m_totalRequestsHandled = 0;
    m_totalResponsesSent = 0;
    m_totalSlaveErrors = 0;
    m_totalExceptionResponses = 0;
    m_fcStats.clear();
}
