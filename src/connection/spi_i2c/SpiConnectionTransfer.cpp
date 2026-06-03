/**
 * @file SpiConnectionTransfer.cpp
 * @brief SPI传输方法实现 — 全双工传输及统计管理
 *
 * 本文件从SpiConnection.cpp拆分而来，包含SPI全双工传输的实现
 * 以及按模式统计的传输计数管理。
 * 传输方法支持默认字长和指定字长两种调用形式。
 *
 * @see SpiConnection.cpp — 核心连接生命周期管理(打开/关闭/配置)
 */

#include "connection/spi_i2c/SpiConnection.h"

/** @brief SPI全双工传输(使用当前配置的默认字长)，同时发送和接收数据 @param txData 发送数据 @return 接收到的MISO数据 */
QByteArray SpiConnection::transfer(const QByteArray& txData)
{
    return transfer(txData, m_wordSize);
}

/** @brief SPI全双工传输(指定字长)，同时发送和接收数据 @param txData 发送数据 @param wordSize 本次传输使用的字长 @return 接收到的MISO数据 */
QByteArray SpiConnection::transfer(const QByteArray& txData, SpiWordSize wordSize)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        return QByteArray();
    }

    m_responseBuffer.clear();

    /// 构建transfer帧，在负载头部添加字长信息
    QByteArray payload;
    /// 字长标识: 0=8bit, 1=16bit, 2=32bit
    char wordSizeCode = 0;
    if (wordSize == SpiWordSize::Bit16) wordSizeCode = 1;
    else if (wordSize == SpiWordSize::Bit32) wordSizeCode = 2;
    payload.append(wordSizeCode);
    payload.append(txData);

    QByteArray frame;
    frame.append(static_cast<char>(CMD_SPI_TRANSFER));
    quint16 len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload);

    m_serial->write(frame);

    /// 解析响应数据(异步缓冲区中提取)
    QByteArray rxData;
    if (!m_responseBuffer.isEmpty()) {
        /// 跳过帧头(CMD+LEN=3字节)，提取有效负载数据
        if (m_responseBuffer.size() > 3) {
            quint16 respLen = static_cast<quint8>(m_responseBuffer[1]) |
                              (static_cast<quint8>(m_responseBuffer[2]) << 8);
            int dataStart = 3;
            int avail = qMin(static_cast<int>(respLen),
                             m_responseBuffer.size() - dataStart);
            if (avail > 0) {
                rxData = m_responseBuffer.mid(dataStart, avail);
            }
        } else {
            rxData = m_responseBuffer;
        }
    }

    /// 更新统计: 全双工同时计发送和接收
    ++m_totalTransfers;
    m_totalBytesSent += static_cast<quint64>(txData.size());
    m_totalBytesReceived += static_cast<quint64>(rxData.size());

    /// 按当前SPI模式累计
    if (m_mode >= 0 && m_mode < 4) {
        ++m_transferByMode[m_mode];
    }

    emit dataReceived(rxData);
    emit transferCompleted(txData.size(), rxData.size());
    return rxData;
}
