/**
 * @file ModbusMaster.cpp
 * @brief Modbus主站（客户端）实现
 *
 * 通过IConnection发送Modbus请求帧（含CRC16），管理超时和响应解析。
 * 支持RTU模式的CRC16校验和多种功能码。
 */
#include "protocol/modbus/ModbusMaster.h"

/** @brief 构造Modbus主站(创建超时定时器) @param parent 父对象 */
ModbusMaster::ModbusMaster(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ModbusMaster::onTimeout);
}

/** @brief 设置数据连接(旧连接自动断开信号) @param connection IConnection指针 */
void ModbusMaster::setConnection(IConnection* connection) {
    if (m_connection) {
        disconnect(m_connection, &IConnection::dataReceived,
                   this, &ModbusMaster::onRawDataReceived);
    }
    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &IConnection::dataReceived,
                this, &ModbusMaster::onRawDataReceived);
    }
}

/** @brief 设置响应超时时间 @param ms 超时毫秒数 */
void ModbusMaster::setTimeout(int ms) {
    m_timeoutMs = ms;
}

/** @brief 发送FC01读线圈请求 @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=发送成功 */
bool ModbusMaster::readCoils(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadCoils;
    frame.startAddress = static_cast<quint16>(start);
    frame.quantity     = static_cast<quint16>(count);
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送FC03读保持寄存器请求 @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=发送成功 */
bool ModbusMaster::readRegisters(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadHoldingRegisters;
    frame.startAddress = static_cast<quint16>(start);
    frame.quantity     = static_cast<quint16>(count);
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送FC06写单个寄存器请求 @param slave 从站地址 @param addr 寄存器地址 @param value 写入值 @return true=发送成功 */
bool ModbusMaster::writeSingleRegister(int slave, int addr, quint16 value) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::WriteSingleRegister;
    frame.startAddress = static_cast<quint16>(addr);
    frame.data.append(static_cast<char>((value >> 8) & 0xFF));
    frame.data.append(static_cast<char>(value & 0xFF));
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送FC16写多个寄存器请求 @param slave 从站地址 @param addr 起始寄存器地址 @param values 写入值列表 @return true=发送成功 */
bool ModbusMaster::writeMultipleRegisters(int slave, int addr,
                                          const QList<quint16>& values) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::WriteMultipleRegisters;
    frame.startAddress = static_cast<quint16>(addr);
    frame.quantity     = static_cast<quint16>(values.size());

    // 字节计数 + 寄存器数据
    QByteArray payload;
    payload.append(static_cast<char>(values.size() * 2));
    for (quint16 val : values) {
        payload.append(static_cast<char>((val >> 8) & 0xFF));
        payload.append(static_cast<char>(val & 0xFF));
    }
    frame.data = payload;
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送自定义Modbus帧 @param frame 自定义帧结构 @return true=发送成功 */
bool ModbusMaster::sendCustomFrame(const ModbusFrame& frame) {
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送原始Modbus帧数据(含CRC16) @param rawData 完整帧字节数组 @return true=写入成功 */
bool ModbusMaster::sendFrame(const QByteArray& rawData) {
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        return false;
    }
    // frameToBytes已包含CRC16校验
    m_lastSlave = static_cast<quint8>(rawData.isEmpty() ? 0 : rawData[0]);
    m_rxBuffer.clear();
    m_timer->start(m_timeoutMs);
    ++m_totalRequests;

    qint64 written = m_connection->write(rawData);
    return written > 0;
}

/** @brief 连接数据到达回调(缓冲+溢出保护+按功能码判断帧长度) @param data 接收到的原始字节 */
void ModbusMaster::onRawDataReceived(const QByteArray& data) {
    m_rxBuffer.append(data);

    /* 防御: 缓冲区过大时截断(防止内存泄漏) */
    if (m_rxBuffer.size() > 256) {
        m_rxBuffer.clear();
        return;
    }

    // 根据功能码判断期望响应帧长度
    if (m_rxBuffer.size() < 2) return;

    quint8 fc = static_cast<quint8>(m_rxBuffer[1]);

    // 异常响应: slave(1) + func|0x80(1) + errCode(1) + CRC(2) = 5
    if (fc & 0x80) {
        if (m_rxBuffer.size() >= 5) {
            m_timer->stop();
            parseResponse(m_rxBuffer.left(5));
            m_rxBuffer.clear();
        }
        return;
    }

    int expectedLen = -1;
    // FC05-06/15-16响应: slave(1)+func(1)+addr(2)+value(2)+CRC(2) = 8
    if (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10) {
        expectedLen = 8;
    }
    // FC01-04响应: slave(1)+func(1)+byteCount(1)+data(N)+CRC(2)
    else if (fc >= 0x01 && fc <= 0x04) {
        if (m_rxBuffer.size() >= 3) {
            int byteCount = static_cast<quint8>(m_rxBuffer[2]);
            expectedLen = 3 + byteCount + 2;  // header + data + CRC
        }
    }

    if (expectedLen > 0 && m_rxBuffer.size() >= expectedLen) {
        m_timer->stop();
        parseResponse(m_rxBuffer.left(expectedLen));
        m_rxBuffer.remove(0, expectedLen);
    }
}

/** @brief 超时回调：递增超时计数并发射timeout信号 */
void ModbusMaster::onTimeout() {
    ++m_totalTimeouts;
    emit timeout(static_cast<int>(m_lastSlave), 0);
}

/** @brief 解析Modbus响应帧(CRC16校验+异常码检测+发射responseReceived/error) @param data 响应帧数据 */
void ModbusMaster::parseResponse(const QByteArray& data) {
    // 校验CRC16（最后2字节为CRC，小端序）
    if (data.size() < 4) { return; }

    QByteArray payload = data.left(data.size() - 2);
    quint16 recvCrc = static_cast<quint8>(data[data.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(
                          data[data.size() - 1])) << 8);
    if (crc16(payload) != recvCrc) {
        // CRC校验失败，丢弃该帧
        return;
    }

    ModbusFrame frame = bytesToFrame(data);
    if (frame.exception) {
        ++m_totalErrors;
        if (!frame.data.isEmpty()) {
            emit error(static_cast<ModbusError>(
                static_cast<quint8>(frame.data[0])));
        }
    } else {
        ++m_totalResponses;
        emit responseReceived(frame);
    }
}

// ============================================================================
// 统计接口（新命名）
// ============================================================================

/** @brief 获取累计发送的请求总数 @return 请求数 */
quint64 ModbusMaster::totalRequests() const
{
    return m_totalRequests;
}

/** @brief 获取累计接收的有效响应总数 @return 响应数 */
quint64 ModbusMaster::totalResponses() const
{
    return m_totalResponses;
}

/** @brief 获取累计超时次数 @return 超时次数 */
quint64 ModbusMaster::totalTimeouts() const
{
    return m_totalTimeouts;
}

/** @brief 获取累计Modbus异常响应总数 @return 错误数 */
quint64 ModbusMaster::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 重置所有统计计数器 */
void ModbusMaster::resetStats()
{
    m_totalRequests = 0;
    m_totalResponses = 0;
    m_totalTimeouts = 0;
    m_totalErrors = 0;
}

// ============================================================================
// 兼容旧接口
// ============================================================================

/** @brief 兼容旧接口: requestCount -> totalRequests */
quint64 ModbusMaster::requestCount() const
{
    return m_totalRequests;
}

/** @brief 兼容旧接口: responseCount -> totalResponses */
quint64 ModbusMaster::responseCount() const
{
    return m_totalResponses;
}

/** @brief 兼容旧接口: timeoutCount -> totalTimeouts */
quint64 ModbusMaster::timeoutCount() const
{
    return m_totalTimeouts;
}

/** @brief 兼容旧接口: errorCount -> totalErrors */
quint64 ModbusMaster::errorCount() const
{
    return m_totalErrors;
}

/** @brief 兼容旧接口: resetStatistics -> resetStats */
void ModbusMaster::resetStatistics()
{
    resetStats();
}
