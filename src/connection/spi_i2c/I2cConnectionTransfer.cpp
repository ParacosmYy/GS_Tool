/**
 * @file I2cConnectionTransfer.cpp
 * @brief I2C总线数据传输/帧构建/响应解析及统计管理实现
 *
 * 本文件从I2cConnection.cpp拆分而来，包含:
 *   - 寄存器读写(readRegister/writeRegister)
 *   - 协议帧发送底层(sendCommand)
 *   - I2C帧构建: 读/写/突发读/突发写帧
 *   - 响应缓冲区解析(parseResponsePayload)
 *   - 统计计数器重置(resetStats)
 *
 * @see I2cConnection.cpp — 核心连接生命周期管理(打开/关闭/配置)
 * @see I2cConnectionScan.cpp — 总线扫描/探测及突发读写
 */

#include "connection/spi_i2c/I2cConnection.h"

#include <QEventLoop>
#include <QTimer>

/** @brief 从指定设备的寄存器读取数据(I2C读时序) @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param length 读取长度 @return 读取到的数据 */
QByteArray I2cConnection::readRegister(int deviceAddr, int regAddr, int length)
{
    QByteArray data;
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        emit registerRead(regAddr, data);
        return data;
    }

    m_responseBuffer.clear();

    /// 构建并发送I2C读命令帧
    QByteArray frame = buildReadFrame(deviceAddr, regAddr, length);
    m_serial->write(frame);

    /// 同步等待响应(最多100ms)
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(this, &IConnection::dataReceived, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(100);
    loop.exec();

    /// 解析响应数据
    QByteArray payload = parseResponsePayload();
    if (!payload.isEmpty()) {
        data = payload;
    }

    /// 更新统计: 读操作
    ++m_totalTransactions;
    m_totalBytesSent += static_cast<quint64>(frame.size());
    m_totalBytesReceived += static_cast<quint64>(data.size());
    m_totalBytesRead += static_cast<quint64>(data.size());

    emit registerRead(regAddr, data);
    return data;
}

/** @brief 向指定设备的寄存器写入数据(I2C写时序) @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param data 待写入数据 @return true=写入成功 */
bool I2cConnection::writeRegister(int deviceAddr, int regAddr, const QByteArray& data)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        return false;
    }

    QByteArray frame = buildWriteFrame(deviceAddr, regAddr, data);
    qint64 written = m_serial->write(frame);

    if (written > 0) {
        ++m_totalTransactions;
        m_totalBytesSent += static_cast<quint64>(written);
        m_totalBytesWritten += static_cast<quint64>(data.size());
    } else {
        ++m_errorCount;
        ++m_totalBusErrors;
    }
    return written > 0;
}

/** @brief 发送协议命令帧[CMD][LEN(2字节小端)][PAYLOAD] @param cmd 命令字节 @param payload 负载数据 @return 发送字节数 */
qint64 I2cConnection::sendCommand(quint8 cmd, const QByteArray& payload)
{
    if (!m_serial) return -1;

    QByteArray frame;
    frame.append(static_cast<char>(cmd));
    quint16 len = static_cast<quint16>(qMin(payload.size(), 65535));
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload.left(len));

    return m_serial->write(frame);
}

/** @brief 构建I2C读命令帧[CMD][LEN][deviceAddr+regAddr+length] @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param length 读取长度 @return 完整协议帧 */
QByteArray I2cConnection::buildReadFrame(int deviceAddr, int regAddr, int length)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(static_cast<char>(length));
    return QByteArray().append(static_cast<char>(CMD_I2C_READ))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C写命令帧[CMD][LEN][deviceAddr+regAddr+data] @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param data 写入数据 @return 完整协议帧 */
QByteArray I2cConnection::buildWriteFrame(int deviceAddr, int regAddr, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(data);
    return QByteArray().append(static_cast<char>(CMD_I2C_WRITE))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C突发读命令帧[CMD][LEN][deviceAddr+startReg+count(2字节小端)] @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param count 读取字节数 @return 完整协议帧 */
QByteArray I2cConnection::buildBurstReadFrame(int deviceAddr, int startReg, int count)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(startReg));
    /// 突发长度用2字节小端表示(支持大于255字节)
    payload.append(static_cast<char>(count & 0xFF));
    payload.append(static_cast<char>((count >> 8) & 0xFF));
    return QByteArray().append(static_cast<char>(CMD_I2C_BURST_RD))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C突发写命令帧[CMD][LEN][deviceAddr+startReg+data] @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param data 待写入的连续数据 @return 完整协议帧 */
QByteArray I2cConnection::buildBurstWriteFrame(int deviceAddr, int startReg, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(startReg));
    payload.append(data);
    return QByteArray().append(static_cast<char>(CMD_I2C_BURST_WR))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 从响应缓冲区解析协议帧的负载数据(跳过CMD+LEN=3字节帧头) @return 负载数据，无效帧返回空 */
QByteArray I2cConnection::parseResponsePayload()
{
    if (m_responseBuffer.size() < 3) {
        return QByteArray();
    }
    quint16 len = static_cast<quint8>(m_responseBuffer[1]) |
                  (static_cast<quint8>(m_responseBuffer[2]) << 8);
    int dataStart = 3;
    int avail = qMin(static_cast<int>(len), m_responseBuffer.size() - dataStart);
    if (avail <= 0) {
        return QByteArray();
    }
    return m_responseBuffer.mid(dataStart, avail);
}

/** @brief 重置所有I2C统计计数器(传输次数/字节数/错误/NACK/总线错误/设备发现/扫描次数/打开次数) */
void I2cConnection::resetStats()
{
    m_totalTransactions = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_errorCount = 0;
    m_nackCount = 0;
    m_totalNacks = 0;
    m_totalBusErrors = 0;
    m_devicesFound = 0;
    m_totalBusScans = 0;
    m_totalOpenAttempts = 0;
    m_lastScanResults.clear();
}
