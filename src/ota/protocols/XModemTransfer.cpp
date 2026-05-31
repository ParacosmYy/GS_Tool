/**
 * @file XModemTransfer.cpp
 * @brief XMODEM协议传输器实现
 *
 * XMODEM Sender端完整实现，包含三种模式支持和增强功能:
 *   - 传输速率实时计算与ETA显示
 *   - 每块独立重试计数(最多10次)
 *   - CAN取消帧发送
 *   - 完善的错误状态转换
 */

#include "ota/protocols/XModemTransfer.h"
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

XModemTransfer::XModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
}

void XModemTransfer::setMode(Mode mode)
{
    m_mode = mode;
}

void XModemTransfer::setFilePath(const QString& path)
{
    m_filePath = path;
}

void XModemTransfer::setData(const QByteArray& data)
{
    m_data = data;
    m_filePath.clear();
}

double XModemTransfer::transferRate() const
{
    return m_currentRate;
}

double XModemTransfer::etaSeconds() const
{
    if (m_currentRate <= 0.0 || m_data.isEmpty()) {
        return -1.0;
    }
    qint64 remaining = m_data.size() - m_bytesSent;
    if (remaining <= 0) {
        return 0.0;
    }
    return static_cast<double>(remaining) / m_currentRate;
}

// ---- BaseTransfer钩子实现 ----

bool XModemTransfer::onStartInit()
{
    // 加载文件数据
    if (m_data.isEmpty() && !m_filePath.isEmpty()) {
        // 文件大小校验: 拒绝超过1MB的文件，防止内存耗尽
        static constexpr qint64 kMaxFileSize = 1024 * 1024; // 1MB
        QFileInfo fileInfo(m_filePath);
        if (fileInfo.size() > kMaxFileSize) {
            qWarning() << "XModem: file too large:" << fileInfo.size()
                       << "bytes (max" << kMaxFileSize << "bytes)";
            emit transferError(tr("文件过大: %1 (%2 字节, 上限 %3 字节)")
                                   .arg(fileInfo.fileName())
                                   .arg(fileInfo.size())
                                   .arg(kMaxFileSize));
            return false;
        }

        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit transferError(tr("无法打开文件: %1").arg(m_filePath));
            return false;
        }
        m_data = file.readAll();
        file.close();
    }

    if (m_data.isEmpty()) {
        emit transferError(tr("无传输数据"));
        return false;
    }

    // 连接有效性检查: 防止空连接下启动传输
    if (!m_conn) {
        emit transferError(tr("传输启动失败: 连接未就绪"));
        return false;
    }

    // 重置传输状态
    m_blockNumber = 1;
    m_bytesSent = 0;
    m_blockRetryCount = 0;
    m_currentRate = 0.0;
    m_lastStatsBytes = 0;
    m_transferTimer.start();

    // 等待接收方发送启动信号(NAK=Checksum模式, C=CRC模式)
    m_xmodemState = State::WaitingForStart;
    m_timeoutTimer->start(m_timeoutMs * 3);
    return true;
}

void XModemTransfer::sendCancelBytes()
{
    // XMODEM取消协议: 连续发送2个CAN字节
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

void XModemTransfer::handleTimeout()
{
    // 超时重发当前状态，每块独立计算重试次数
    if (m_xmodemState == State::SendingBlock) {
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            // 单块重试10次失败，发送CAN取消传输
            sendCancelBytes();
            m_xmodemState = State::Error;
            markError();
            emit transferError(
                tr("块 %1 超时: 重试次数耗尽 (10次), 已传输 %2/%3 字节")
                    .arg(m_blockNumber)
                    .arg(m_bytesSent)
                    .arg(m_data.size()));
            return;
        }
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::SendingEOT) {
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            sendCancelBytes();
            m_xmodemState = State::Error;
            markError();
            emit transferError(tr("EOT确认超时: 重试次数耗尽 (10次)"));
            return;
        }
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::WaitingForStart) {
        // 等待启动信号时使用较长超时
        m_timeoutTimer->start(m_timeoutMs * 3);
    }
}

void XModemTransfer::processReceivedData()
{
    int readIdx = 0;
    const int len = m_receiveBuffer.size();

    while (readIdx < len) {
        char ch = m_receiveBuffer.at(readIdx);
        readIdx++;

        if (m_cancelled) {
            m_receiveBuffer.remove(0, readIdx);
            return;
        }

        switch (m_xmodemState) {
        case State::Idle:
        case State::Done:
        case State::Error:
            m_receiveBuffer.remove(0, readIdx);
            return;

        case State::WaitingForStart:
            if (ch == NAK) {
                // 接收方请求Checksum模式 — 但不覆盖用户显式选择的模式
                // 仅当用户未显式选择(默认CRC)且接收方只支持Checksum时才降级
                // NAK 在 SendingBlock 阶段仍然按协议处理(块重发)
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_xmodemState = State::SendingBlock;
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CRC_CHAR) {
                // 接收方请求CRC模式
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_xmodemState = State::SendingBlock;
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingBlock:
            if (ch == ACK) {
                // 块确认成功，重置重试计数
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_blockNumber++;
                if (m_blockNumber > 255) m_blockNumber = 1;

                // 更新速率统计
                updateTransferStats();

                int percent = static_cast<int>((m_bytesSent * 100) / m_data.size());
                emit progress(percent, m_bytesSent, m_data.size());

                if (m_bytesSent >= m_data.size()) {
                    // 所有数据已发送，发送EOT
                    m_xmodemState = State::SendingEOT;
                    m_blockRetryCount = 0;
                    sendEOT();
                    m_timeoutTimer->start(m_timeoutMs);
                } else {
                    sendBlock();
                    m_timeoutTimer->start(m_timeoutMs);
                }
            } else if (ch == NAK) {
                // 块被拒绝(CRC/校验和错误)，重发当前块
                m_timeoutTimer->stop();
                m_blockRetryCount++;
                if (m_blockRetryCount > 10) {
                    sendCancelBytes();
                    m_xmodemState = State::Error;
                    markError();
                    emit transferError(
                        tr("块 %1 CRC校验失败: 被接收方拒绝次数过多 (10次), 已传输 %2/%3 字节")
                            .arg(m_blockNumber)
                            .arg(m_bytesSent)
                            .arg(m_data.size()));
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                // 接收方取消传输
                m_timeoutTimer->stop();
                m_xmodemState = State::Error;
                markError();
                emit transferError(
                    tr("接收方取消传输, 已传输 %1/%2 字节 (块 %3)")
                        .arg(m_bytesSent)
                        .arg(m_data.size())
                        .arg(m_blockNumber));
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingEOT:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                emit progress(100, m_data.size(), m_data.size());
                m_xmodemState = State::Done;
                finishTransfer();
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_blockRetryCount++;
                if (m_blockRetryCount > 10) {
                    sendCancelBytes();
                    m_xmodemState = State::Error;
                    markError();
                    emit transferError(tr("EOT被拒绝: 重试次数耗尽 (10次)"));
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendEOT();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_xmodemState = State::Error;
                markError();
                emit transferError(tr("接收方在EOT阶段取消传输, 已传输 %1/%2 字节")
                                       .arg(m_bytesSent).arg(m_data.size()));
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;
        }
    }

    // 单次O(n)压缩，替代循环中每次O(n)的remove
    m_receiveBuffer.remove(0, readIdx);
}

// ---- 数据发送 ----

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
    m_bytesSent = qMin(offset + dataSize, static_cast<qint64>(m_data.size()));
}

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

quint16 XModemTransfer::xmodemCrc(const QByteArray& data)
{
    return CRC::crc16Xmodem(data);
}

// ---- 速率统计 ----

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

void XModemTransfer::setState(State newState)
{
    m_xmodemState = newState;
}
