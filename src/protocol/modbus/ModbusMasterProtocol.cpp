/**
 * @file ModbusMasterProtocol.cpp
 * @brief Modbus主站 -- 帧发送、接收缓冲、响应解析实现
 *
 * 从 ModbusMaster.cpp 拆分而来，包含:
 * - sendFrame: 发送原始帧(RTU走IConnection, TCP走QTcpSocket)
 * - onRawDataReceived: RTU模式串口数据接收与帧定界
 * - onTcpReadyRead: TCP模式数据接收与MBAP帧定界
 * - parseRtuResponse/parseTcpResponse: CRC校验与响应解析
 * - handleResponse: 通用响应处理(统计+信号发射)
 */

#include "protocol/modbus/ModbusMaster.h"

// ============================================================================
// 帧发送
// ============================================================================

/**
 * @brief 发送原始帧数据 -- 根据模式选择RTU串口或TCP套接字
 * @param rawData 完整ADU字节数组
 * @return true=写入成功
 */
bool ModbusMaster::sendFrame(const QByteArray& rawData) {
    /* 记录本次功能码(用于超时统计) */
    if (m_mode == Mode::RTU && rawData.size() >= 2) {
        m_lastFunction = static_cast<quint8>(rawData[1]);
    } else if (m_mode == Mode::TCP && rawData.size() >= 8) {
        /* TCP: MBAP(7字节)后第1字节是功能码 */
        m_lastFunction = static_cast<quint8>(rawData[7]);
    }

    m_rxBuffer.clear();
    m_timer->start(m_timeoutMs);
    ++m_totalRequests;
    ++m_fcStats[static_cast<int>(m_lastFunction)];

    qint64 written = 0;
    if (m_mode == Mode::RTU) {
        /* RTU模式: 通过IConnection写入串口 */
        if (!m_connection || m_connection->state() != ConnectionState::Connected) {
            m_timer->stop();
            emit communicationError(tr("RTU连接未就绪"));
            return false;
        }
        written = m_connection->write(rawData);
    } else {
        /* TCP模式: 通过QTcpSocket写入 */
        if (!m_tcpSocket || m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
            /* 尝试自动连接 */
            m_tcpSocket->connectToHost(m_tcpHost, m_tcpPort);
            if (!m_tcpSocket->waitForConnected(m_timeoutMs)) {
                m_timer->stop();
                emit communicationError(tr("TCP连接失败: %1:%2")
                    .arg(m_tcpHost).arg(m_tcpPort));
                return false;
            }
        }
        written = m_tcpSocket->write(rawData);
    }

    if (written > 0) {
        m_bytesTransmitted += static_cast<quint64>(written);
        return true;
    }
    m_timer->stop();
    return false;
}

// ============================================================================
// RTU模式接收
// ============================================================================

/** @brief RTU串口数据到达: 缓冲+溢出保护+按功能码判断帧长度 */
void ModbusMaster::onRawDataReceived(const QByteArray& data) {
    m_rxBuffer.append(data);
    m_bytesReceived += static_cast<quint64>(data.size());

    /* 防御: 缓冲区过大时截断(最大合法Modbus帧256字节,留余量512) */
    if (m_rxBuffer.size() > 512) {
        m_rxBuffer.clear();
        return;
    }

    if (m_rxBuffer.size() < 2) return;

    quint8 fc = static_cast<quint8>(m_rxBuffer[1]);

    /* 异常响应: slave(1)+func|0x80(1)+errCode(1)+CRC(2) = 5字节 */
    if (fc & 0x80) {
        if (m_rxBuffer.size() >= 5) {
            m_timer->stop();
            parseRtuResponse(m_rxBuffer.left(5));
            m_rxBuffer.clear();
        }
        return;
    }

    int expectedLen = -1;
    /* FC05/06/15/16响应: slave(1)+func(1)+addr(2)+value(2)+CRC(2) = 8 */
    if (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10) {
        expectedLen = 8;
    }
    /* FC01-04响应: slave(1)+func(1)+byteCount(1)+data(N)+CRC(2) */
    else if (fc >= 0x01 && fc <= 0x04) {
        if (m_rxBuffer.size() >= 3) {
            int byteCount = static_cast<quint8>(m_rxBuffer[2]);
            expectedLen = 3 + byteCount + 2;
        }
    }

    if (expectedLen > 0 && m_rxBuffer.size() >= expectedLen) {
        m_timer->stop();
        parseRtuResponse(m_rxBuffer.left(expectedLen));
        m_rxBuffer.remove(0, expectedLen);
    }
}

// ============================================================================
// TCP模式接收
// ============================================================================

/** @brief TCP数据到达: 按MBAP头长度字段定界，匹配事务ID */
void ModbusMaster::onTcpReadyRead() {
    m_rxBuffer.append(m_tcpSocket->readAll());

    /* MBAP头最小7字节: 事务ID(2)+协议ID(2)+长度(2)+单元ID(1) */
    while (m_rxBuffer.size() >= 7) {
        /* 协议ID必须为0 */
        quint16 protoId = (static_cast<quint8>(m_rxBuffer[2]) << 8) |
                           static_cast<quint8>(m_rxBuffer[3]);
        if (protoId != 0) {
            m_rxBuffer.clear();
            return;
        }

        /* 长度字段: 后续字节数(单元ID+PDU) */
        quint16 length = (static_cast<quint8>(m_rxBuffer[4]) << 8) |
                          static_cast<quint8>(m_rxBuffer[5]);
        int totalFrameLen = 6 + length; /* MBAP头(6)+长度字段指示的字节数 */

        if (m_rxBuffer.size() < totalFrameLen) return; /* 数据不完整 */

        m_bytesReceived += static_cast<quint64>(totalFrameLen);

        QByteArray frame = m_rxBuffer.left(totalFrameLen);
        m_rxBuffer.remove(0, totalFrameLen);

        /* 校验事务ID匹配 */
        quint16 recvTxId = (static_cast<quint8>(frame[0]) << 8) |
                            static_cast<quint8>(frame[1]);
        if (recvTxId != m_lastTxId) {
            /* 事务ID不匹配 -- 丢弃 */
            continue;
        }

        m_timer->stop();
        parseTcpResponse(frame);
    }
}

// ============================================================================
// 超时处理
// ============================================================================

/** @brief 超时回调: 按读写分类递增失败计数并发射信号 */
void ModbusMaster::onTimeout() {
    ++m_totalTimeouts;
    ++m_totalRetries;
    if (isReadFunction(m_lastFunction)) {
        ++m_failedReads;
    } else if (isWriteFunction(m_lastFunction)) {
        ++m_failedWrites;
    }
    emit timeoutOccurred(m_lastFunction);
    /* 兼容旧信号 */
    emit timeout(static_cast<int>(m_slaveAddress), static_cast<int>(m_lastFunction));
}

// ============================================================================
// RTU响应解析
// ============================================================================

/** @brief 解析RTU响应帧: CRC16校验+提取PDU数据 @param data 完整RTU帧(含CRC) */
void ModbusMaster::parseRtuResponse(const QByteArray& data) {
    if (data.size() < 4) return;

    /* CRC校验: 数据部分 vs 尾部2字节CRC */
    QByteArray payload = data.left(data.size() - 2);
    quint16 recvCrc = static_cast<quint8>(data[data.size() - 2]) |
                      (static_cast<quint16>(static_cast<quint8>(
                          data[data.size() - 1])) << 8);
    if (crc16(payload) != recvCrc) {
        ++m_totalCrcErrors;
        if (isReadFunction(m_lastFunction)) ++m_failedReads;
        else if (isWriteFunction(m_lastFunction)) ++m_failedWrites;
        emit communicationError(tr("CRC校验失败"));
        return;
    }

    /* 从RTU帧中提取PDU部分(去掉slave地址和CRC) */
    QByteArray pdu = payload.mid(1); /* 去掉slave地址 */
    ModbusFrame frame = bytesToFrame(data);
    frame.data = pdu.mid(1); /* PDU数据部分(去掉功能码) */

    /* 对于读操作响应, 需要提取实际数据 */
    quint8 fc = static_cast<quint8>(frame.function);
    if (isReadFunction(fc) && pdu.size() >= 2) {
        quint8 byteCount = static_cast<quint8>(pdu[1]);
        if (pdu.size() >= 2 + byteCount) {
            frame.data = pdu.mid(2, byteCount);
        }
    }

    handleResponse(frame);
}

// ============================================================================
// TCP响应解析
// ============================================================================

/** @brief 解析TCP响应帧: MBAP头校验+提取PDU @param data 完整MBAP+PDU */
void ModbusMaster::parseTcpResponse(const QByteArray& data) {
    /* MBAP: 事务ID(2)+协议ID(2)+长度(2)+单元ID(1) = 7字节最小 */
    if (data.size() < 9) return; /* 7 + func(1) + 最少1字节数据 */

    quint8 unitId = static_cast<quint8>(data[6]);
    Q_UNUSED(unitId)

    QByteArray pdu = data.mid(7); /* PDU: 功能码(1)+数据 */
    if (pdu.isEmpty()) return;

    quint8 fc = static_cast<quint8>(pdu[0]);

    ModbusFrame frame;
    frame.slaveAddress = m_slaveAddress;
    frame.startAddress = 0;
    frame.quantity = 0;

    /* 异常响应: 功能码最高位为1 */
    if (fc & 0x80) {
        frame.exception = true;
        frame.function = static_cast<ModbusFunction>(fc & 0x7F);
        if (pdu.size() >= 2) frame.data = pdu.mid(1);
        handleResponse(frame);
        return;
    }

    frame.function = static_cast<ModbusFunction>(fc);

    if (isReadFunction(fc)) {
        /* 读响应: func(1)+byteCount(1)+data(N) */
        if (pdu.size() >= 2) {
            quint8 byteCount = static_cast<quint8>(pdu[1]);
            if (pdu.size() >= 2 + byteCount) {
                frame.data = pdu.mid(2, byteCount);
            }
        }
    } else if (fc == 0x05 || fc == 0x06) {
        /* 写单个响应: func(1)+addr(2)+value(2) */
        if (pdu.size() >= 5) {
            frame.startAddress = (static_cast<quint8>(pdu[1]) << 8) |
                                  static_cast<quint8>(pdu[2]);
            frame.data = pdu.mid(3);
        }
    } else if (fc == 0x0F || fc == 0x10) {
        /* 写多个响应: func(1)+addr(2)+qty(2) */
        if (pdu.size() >= 5) {
            frame.startAddress = (static_cast<quint8>(pdu[1]) << 8) |
                                  static_cast<quint8>(pdu[2]);
            frame.quantity = (static_cast<quint8>(pdu[3]) << 8) |
                              static_cast<quint8>(pdu[4]);
        }
    }

    handleResponse(frame);
}

// ============================================================================
// 通用响应处理
// ============================================================================

/** @brief 通用响应处理: 更新统计+发射信号(新/旧双接口) @param frame 解析后的帧 */
void ModbusMaster::handleResponse(const ModbusFrame& frame) {
    quint8 fc = static_cast<quint8>(frame.function);

    if (frame.exception) {
        ++m_totalErrors;
        ++m_totalExceptions;
        if (isReadFunction(fc)) ++m_failedReads;
        else if (isWriteFunction(fc)) ++m_failedWrites;
        if (!frame.data.isEmpty()) {
            emit error(static_cast<ModbusError>(static_cast<quint8>(frame.data[0])));
        }
        emit communicationError(tr("Modbus异常响应: FC%1, 错误码=%2")
            .arg(fc).arg(frame.data.isEmpty() ? 0 : static_cast<int>(frame.data[0])));
        return;
    }

    ++m_totalResponses;
    if (isReadFunction(fc)) ++m_successfulReads;
    else if (isWriteFunction(fc)) ++m_successfulWrites;

    /* 新信号: PDU数据+功能码 */
    emit responseReceived(frame.data, fc);
    /* 兼容旧信号: 完整ModbusFrame */
    emit responseReceived(frame);
}

// ============================================================================
// 辅助方法
// ============================================================================

/** @brief 功能码转统计数组索引 [0..7] */
int ModbusMaster::fcToIndex(quint8 fc) {
    switch (fc) {
    case 0x01: return 0;
    case 0x02: return 1;
    case 0x03: return 2;
    case 0x04: return 3;
    case 0x05: return 4;
    case 0x06: return 5;
    case 0x0F: return 6;
    case 0x10: return 7;
    default:   return 0;
    }
}

/** @brief 判断是否为读操作(FC01-04) @param fc 功能码 */
bool ModbusMaster::isReadFunction(quint8 fc) {
    return (fc >= 0x01 && fc <= 0x04);
}

/** @brief 判断是否为写操作(FC05/06/15/16) @param fc 功能码 */
bool ModbusMaster::isWriteFunction(quint8 fc) {
    return (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10);
}
