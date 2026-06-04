/**
 * @file ModbusMasterProtocol.cpp
 * @brief Modbus主站 - 帧发送与响应解析实现
 *
 * 从 ModbusMaster.cpp 拆分而来，包含帧发送、原始数据接收缓冲、
 * 超时处理、CRC校验响应解析和功能码分类判断方法。
 */

#include "protocol/modbus/ModbusMaster.h"

/** @brief 发送原始Modbus帧数据(含CRC16+记录功能码+启动超时定时器) @param rawData 完整帧字节数组 @return true=写入成功 */
bool ModbusMaster::sendFrame(const QByteArray& rawData) {
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        return false;
    }
    // 记录本次请求的从站地址和功能码（用于超时统计分类）
    if (rawData.size() >= 2) {
        m_lastSlave    = static_cast<quint8>(rawData[0]);
        m_lastFunction = static_cast<quint8>(rawData[1]);
    }
    m_rxBuffer.clear();
    m_timer->start(m_timeoutMs);
    ++m_totalRequests;
    ++m_fcStats[static_cast<int>(m_lastFunction)];

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
            expectedLen = 3 + byteCount + 2;
        }
    }

    if (expectedLen > 0 && m_rxBuffer.size() >= expectedLen) {
        m_timer->stop();
        parseResponse(m_rxBuffer.left(expectedLen));
        m_rxBuffer.remove(0, expectedLen);
    }
}

/** @brief 超时回调：按读写类型分类递增失败计数并发射timeout信号 */
void ModbusMaster::onTimeout() {
    ++m_totalTimeouts;
    ++m_totalRetries;
    if (isReadFunction(m_lastFunction)) {
        ++m_failedReads;
    } else if (isWriteFunction(m_lastFunction)) {
        ++m_failedWrites;
    }
    emit timeout(static_cast<int>(m_lastSlave),
                 static_cast<int>(m_lastFunction));
}

/** @brief 解析Modbus响应帧(CRC16校验+异常码检测+按读写分类统计+发射信号) @param data 响应帧数据 */
void ModbusMaster::parseResponse(const QByteArray& data) {
    if (data.size() < 4) { return; }

    QByteArray payload = data.left(data.size() - 2);
    quint16 recvCrc = static_cast<quint8>(data[data.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(
                          data[data.size() - 1])) << 8);
    if (crc16(payload) != recvCrc) {
        if (isReadFunction(m_lastFunction)) {
            ++m_failedReads;
        } else if (isWriteFunction(m_lastFunction)) {
            ++m_failedWrites;
        }
        return;
    }

    ModbusFrame frame = bytesToFrame(data);
    quint8 fc = static_cast<quint8>(frame.function);

    if (frame.exception) {
        ++m_totalErrors;
        ++m_totalExceptions;
        if (isReadFunction(fc)) {
            ++m_failedReads;
        } else if (isWriteFunction(fc)) {
            ++m_failedWrites;
        }
        if (!frame.data.isEmpty()) {
            emit error(static_cast<ModbusError>(
                static_cast<quint8>(frame.data[0])));
        }
    } else {
        ++m_totalResponses;
        if (isReadFunction(fc)) {
            ++m_successfulReads;
        } else if (isWriteFunction(fc)) {
            ++m_successfulWrites;
        }
        emit responseReceived(frame);
    }
}

/** @brief 判断功能码是否为读操作(FC01-04) @param fc 功能码 @return true=读操作 */
bool ModbusMaster::isReadFunction(quint8 fc) {
    return (fc >= 0x01 && fc <= 0x04);
}

/** @brief 判断功能码是否为写操作(FC05/06/15/16) @param fc 功能码 @return true=写操作 */
bool ModbusMaster::isWriteFunction(quint8 fc) {
    return (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10);
}
