/**
 * @file ModbusMaster.cpp
 * @brief Modbus主站（客户端）实现
 *
 * 通过IConnection发送Modbus请求帧（含CRC16），管理超时和响应解析。
 * 支持RTU模式的CRC16校验和多种功能码，按读写类型独立统计。
 *
 * 帧发送与响应解析见：@see ModbusMasterProtocol.cpp
 * 统计接口见：@see ModbusMasterStats.cpp
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
    m_timeoutMs = qMax(10, ms);
}

/** @brief 获取当前响应超时时间 @return 超时毫秒数 */
int ModbusMaster::timeout() const {
    return m_timeoutMs;
}

// ============================================================================
// 功能码便捷方法
// ============================================================================

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
bool ModbusMaster::readHoldingRegisters(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadHoldingRegisters;
    frame.startAddress = static_cast<quint16>(start);
    frame.quantity     = static_cast<quint16>(count);
    return sendFrame(frameToBytes(frame));
}

/** @brief 发送FC04读输入寄存器请求 @param slave 从站地址 @param start 起始地址 @param count 读取数量 @return true=发送成功 */
bool ModbusMaster::readInputRegisters(int slave, int start, int count) {
    ModbusFrame frame;
    frame.slaveAddress = static_cast<quint8>(slave);
    frame.function     = ModbusFunction::ReadInputRegisters;
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
    if (values.isEmpty() || values.size() > 123) return false;
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

// sendFrame/onRawDataReceived/onTimeout/parseResponse/isReadFunction/isWriteFunction
// 已移至 ModbusMasterProtocol.cpp
