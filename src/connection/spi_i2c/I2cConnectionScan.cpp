/**
 * @file I2cConnectionScan.cpp
 * @brief I2C总线扫描/突发读写及探测方法实现
 *
 * 本文件从I2cConnection.cpp拆分而来，包含:
 *   - 总线扫描(7位地址0x03~0x77逐地址ACK探测)
 *   - 单地址ACK探测
 *   - 突发读取/突发写入
 * 总线扫描采用逐地址探测策略，每个地址发送PROBE命令，
 * 根据适配器响应判断设备是否存在(ACK/NACK)。
 *
 * @see I2cConnection.cpp — 核心连接生命周期管理(打开/关闭/配置)
 */

#include "connection/spi_i2c/I2cConnection.h"

/** @brief 扫描I2C总线，逐地址探测0x03~0x77范围内的设备 @return 发现的设备地址列表(7位地址) */
QList<int> I2cConnection::scanBus()
{
    QList<int> found;
    if (m_state != ConnectionState::Connected || !m_serial) {
        return found;
    }

    ++m_totalBusScans;  // 累计总线扫描次数
    m_lastScanResults.clear();

    /// 逐地址执行ACK探测
    for (int addr = SCAN_ADDR_START; addr <= SCAN_ADDR_END; ++addr) {
        if (probeAddress(addr)) {
            found.append(addr);
            m_lastScanResults.append(addr);
            emit deviceFound(addr);
        }
    }

    /// 更新统计: 扫描结果
    m_devicesFound += static_cast<quint64>(found.size());

    emit scanComplete(found);
    return found;
}

/** @brief 对单个I2C地址执行ACK探测 @param addr 7位设备地址 @return true=设备响应ACK，false=NACK或无响应 */
bool I2cConnection::probeAddress(int addr)
{
    if (!m_serial || m_state != ConnectionState::Connected) {
        return false;
    }

    m_responseBuffer.clear();

    /// 构建探测命令帧: [CMD_I2C_PROBE][LEN=1][addr]
    QByteArray probeFrame;
    probeFrame.append(static_cast<char>(CMD_I2C_PROBE));
    probeFrame.append(static_cast<char>(0x01));
    probeFrame.append(static_cast<char>(0x00));
    probeFrame.append(static_cast<char>(addr & 0x7F));

    m_serial->write(probeFrame);

    /// 解析探测响应: 1字节状态(0x00=NACK, 0x01=ACK)
    if (m_responseBuffer.size() >= 4) {
        quint8 status = static_cast<quint8>(m_responseBuffer[3]);
        if (status == 0x01) {
            /// 设备响应ACK
            ++m_totalTransactions;
            m_totalBytesSent += 4;  ///< 探测帧长度
            return true;
        } else {
            /// 设备NACK
            ++m_nackCount;
            ++m_totalNacks;
            ++m_totalTransactions;
            m_totalBytesSent += 4;
            emit nackReceived(addr);
            return false;
        }
    }

    /// 无有效响应，视为通信错误
    ++m_errorCount;
    ++m_totalBusErrors;
    return false;
}

/** @brief 突发读取: 从起始寄存器连续读取多个字节(自动递增地址) @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param count 读取字节数 @return 读取到的连续数据 */
QByteArray I2cConnection::burstRead(int deviceAddr, int startReg, int count)
{
    QByteArray data;
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        emit burstReadComplete(startReg, data);
        return data;
    }

    m_responseBuffer.clear();

    /// 构建并发送突发读命令帧
    QByteArray frame = buildBurstReadFrame(deviceAddr, startReg, count);
    m_serial->write(frame);

    /// 解析响应数据
    QByteArray payload = parseResponsePayload();
    if (!payload.isEmpty()) {
        data = payload;
    }

    /// 更新统计
    ++m_totalTransactions;
    m_totalBytesSent += static_cast<quint64>(frame.size());
    m_totalBytesReceived += static_cast<quint64>(data.size());
    m_totalBytesRead += static_cast<quint64>(data.size());

    emit burstReadComplete(startReg, data);
    return data;
}

/** @brief 突发写入: 从起始寄存器连续写入多个字节(自动递增地址) @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param data 待写入的连续数据 @return true=写入成功 */
bool I2cConnection::burstWrite(int deviceAddr, int startReg, const QByteArray& data)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        return false;
    }

    QByteArray frame = buildBurstWriteFrame(deviceAddr, startReg, data);
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
