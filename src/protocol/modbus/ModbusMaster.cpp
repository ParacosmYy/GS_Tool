/**
 * @file ModbusMaster.cpp
 * @brief Modbus主站（客户端）实现
 *
 * 通过IConnection发送Modbus请求帧，管理超时和响应解析。
 * TODO: 实现CRC16校验、RTU/ASCII帧封装、响应匹配逻辑。
 */
#include "protocol/modbus/ModbusMaster.h"

ModbusMaster::ModbusMaster(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ModbusMaster::onTimeout);
}

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

void ModbusMaster::setTimeout(int ms) {
    m_timeoutMs = ms;
}

bool ModbusMaster::readCoils(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadCoils;
    frame.startAddress = static_cast<quint16>(start);
    frame.quantity     = static_cast<quint16>(count);
    return sendFrame(frameToBytes(frame));
}

bool ModbusMaster::readRegisters(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadHoldingRegisters;
    frame.startAddress = static_cast<quint16>(start);
    frame.quantity     = static_cast<quint16>(count);
    return sendFrame(frameToBytes(frame));
}

bool ModbusMaster::writeSingleRegister(int slave, int addr, quint16 value) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::WriteSingleRegister;
    frame.startAddress = static_cast<quint16>(addr);
    frame.data.append(static_cast<char>((value >> 8) & 0xFF));
    frame.data.append(static_cast<char>(value & 0xFF));
    return sendFrame(frameToBytes(frame));
}

bool ModbusMaster::writeMultipleRegisters(int slave, int addr,
                                          const QList<quint16>& values) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::WriteMultipleRegisters;
    frame.startAddress = static_cast<quint16>(addr);
    frame.quantity     = static_cast<quint16>(values.size());

    // 字节数 + 寄存器数据
    QByteArray payload;
    payload.append(static_cast<char>(values.size() * 2));
    for (quint16 val : values) {
        payload.append(static_cast<char>((val >> 8) & 0xFF));
        payload.append(static_cast<char>(val & 0xFF));
    }
    frame.data = payload;
    return sendFrame(frameToBytes(frame));
}

bool ModbusMaster::sendCustomFrame(const ModbusFrame& frame) {
    return sendFrame(frameToBytes(frame));
}

bool ModbusMaster::sendFrame(const QByteArray& rawData) {
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        return false;
    }
    // TODO: 添加CRC16校验
    m_lastSlave = static_cast<quint8>(rawData.isEmpty() ? 0 : rawData[0]);
    m_rxBuffer.clear();
    m_timer->start(m_timeoutMs);

    qint64 written = m_connection->write(rawData);
    return written > 0;
}

void ModbusMaster::onRawDataReceived(const QByteArray& data) {
    m_rxBuffer.append(data);
    // TODO: 实现完整的帧边界检测逻辑
    // 简化处理：收到足够数据后尝试解析
    if (m_rxBuffer.size() >= 5) {
        m_timer->stop();
        parseResponse(m_rxBuffer);
        m_rxBuffer.clear();
    }
}

void ModbusMaster::onTimeout() {
    emit timeout(static_cast<int>(m_lastSlave), 0);
}

void ModbusMaster::parseResponse(const QByteArray& data) {
    // TODO: 校验CRC16
    ModbusFrame frame = bytesToFrame(data);
    if (frame.exception) {
        if (!frame.data.isEmpty()) {
            emit error(static_cast<ModbusError>(
                static_cast<quint8>(frame.data[0])));
        }
    } else {
        emit responseReceived(frame);
    }
}
