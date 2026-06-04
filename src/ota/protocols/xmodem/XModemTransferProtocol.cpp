/**
 * @file XModemTransferProtocol.cpp
 * @brief XMODEM协议数据包构建、发送与统计方法
 *
 * 从XModemTransfer.cpp拆分，包含数据块构建、CRC校验、
 * EOT发送和传输速率统计等协议辅助方法。
 * 协议状态处理方法见XModemTransferHandlers.cpp。
 */

#include "ota/protocols/xmodem/XModemTransfer.h"

// ---- 数据包构建与发送 ----

/** @brief 构建完整数据块包(帧头+块号+反码+数据+校验)
 *  @param blockNum 块编号(1-255循环)
 *  @param blockData 块数据载荷
 *  @return 完整的数据包字节数组 */
QByteArray XModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;

    // 头部: SOH(128B)或STX(1024B)
    packet.append((m_mode == OneK) ? STX : SOH);

    // 块号: blockNum(1-255循环) + 反码
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF));

    // 数据载荷
    packet.append(blockData);

    // 校验: Checksum模式用算术和，CRC/1K模式用CRC16
    if (m_mode == Checksum) {
        packet.append(static_cast<char>(CRC::checksum(blockData)));
    } else {
        quint16 crc = xmodemCrc(blockData);
        packet.append(static_cast<char>((crc >> 8) & 0xFF));
        packet.append(static_cast<char>(crc & 0xFF));
    }

    return packet;
}

/** @brief 计算XMODEM CRC16校验值 @param data 待校验数据 @return CRC16校验值 */
quint16 XModemTransfer::xmodemCrc(const QByteArray& data)
{
    return CRC::crc16Xmodem(data);
}

/** @brief 构建并发送一个数据块，不足块大小时用0x1A填充 */
void XModemTransfer::sendBlock()
{
    int bs = blockSize();
    // 使用累计已发送字节数作为偏移，而非从块号反算。
    // XModem 块号在 1-255 间循环，不能用于计算文件偏移。
    qint64 offset = m_bytesSent;
    int dataSize = qMin(static_cast<int>(m_data.size() - offset), bs);

    if (dataSize <= 0) {
        // 数据已全部发送，切换到EOT阶段
        m_xmodemState = State::SendingEOT;
        m_blockRetryCount = 0;
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
        return;
    }

    // 提取块数据，不足块大小时用0x1A(Ctrl+Z)填充
    QByteArray blockData = m_data.mid(offset, dataSize);
    if (blockData.size() < bs) {
        blockData.append(QByteArray(bs - blockData.size(), 0x1A));
    }

    QByteArray packet = buildBlock(m_blockNumber, blockData);
    if (m_conn) {
        qint64 written = m_conn->write(packet);
        // 检测连接断开: write返回-1表示连接已不可用
        if (written < 0) {
            m_xmodemState = State::Error;
            markError();
            emit transferError(
                tr("连接中断: 写入失败, 已传输 %1/%2 字节 (块 %3)")
                    .arg(m_bytesSent)
                    .arg(m_data.size())
                    .arg(m_blockNumber));
            return;
        }
    }
    // 注意: m_bytesSent 不在sendBlock中推进，而是在ACK确认后推进，
    // 确保NAK重传时发送相同的块（防止跳块导致数据损坏）
}

/** @brief 发送EOT(End of Transmission)字节通知接收方传输结束 */
void XModemTransfer::sendEOT()
{
    if (m_conn) {
        qint64 written = m_conn->write(QByteArray(1, EOT));
        // 检测连接断开
        if (written < 0) {
            m_xmodemState = State::Error;
            markError();
            emit transferError(
                tr("连接中断: EOT发送失败, 已传输 %1/%2 字节")
                    .arg(m_bytesSent).arg(m_data.size()));
        }
    }
}

// ---- 速率统计 ----

/** @brief 更新传输速率统计并发射transferStats信号 */
void XModemTransfer::updateTransferStats()
{
    qint64 elapsedMs = m_transferTimer.elapsed();
    if (elapsedMs <= 0) return;

    // 基于总传输量计算平均速率
    qint64 totalSent = m_bytesSent;
    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;

    if (elapsedSec > 0.0) {
        m_currentRate = static_cast<double>(totalSent) / elapsedSec;
    }

    // 计算ETA并发射统计信号
    double eta = etaSeconds();
    emit transferStats(m_currentRate, eta);

    m_lastStatsBytes = m_bytesSent;
}

/** @brief 获取累计发送的数据块总数 @return 块数 */
quint64 XModemTransfer::totalBlocksSent() const { return m_totalBlocksSent; }

/** @brief 获取累计重试次数 @return 重试次数 */
quint64 XModemTransfer::totalRetries() const { return m_totalRetries; }

/** @brief 获取累计模式切换次数 @return 模式切换次数 */
quint64 XModemTransfer::totalModeSwitches() const { return m_totalModeSwitches; }

/** @brief 获取累计XMODEM协议错误次数 @return 错误次数 */
quint64 XModemTransfer::xmodemErrorCount() const { return m_xmodemErrorCount; }

/** @brief 重置所有XMODEM传输统计计数器(块数/重试/模式切换/错误/CRC错误/超时) */
void XModemTransfer::resetXmodemStatistics()
{
    m_totalBlocksSent = 0;
    m_totalRetries = 0;
    m_totalModeSwitches = 0;
    m_xmodemErrorCount = 0;
    m_totalCrcErrors = 0;
    m_totalTimeouts = 0;
    m_totalNakReceived = 0;
    m_totalCanReceived = 0;
    m_totalAcksReceived = 0;
}

/** @brief 设置XMODEM状态机状态 @param newState 目标状态 */
void XModemTransfer::setState(State newState)
{
    m_xmodemState = newState;
}
