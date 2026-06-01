/**
 * @file XModemTransferHandlers.cpp
 * @brief XMODEM协议状态处理器 - 协议状态机的各状态处理方法
 *
 * 从XModemTransfer.cpp拆分，包含3个状态处理方法和超时处理。
 * 由processReceivedData()逐字节解析后根据协议状态分发调用。
 */

#include "ota/protocols/XModemTransfer.h"

/** @brief 处理WaitingForStart状态: 解析NAK/C启动信号，自动降级模式
 *  @param ch 接收到的单字节 */
void XModemTransfer::handleStateWaitingForStart(char ch)
{
    if (ch == NAK) {
        // XMODEM协议规范: NAK表示接收方仅支持Checksum模式
        // 当发送方配置为CRC/1K时，必须自动降级为Checksum模式
        // 这是与仅支持Checksum的STM32 bootloader通信的必要兼容措施
        if (m_mode == CRC || m_mode == OneK) {
            QString fromName = (m_mode == OneK)
                ? tr("XMODEM-1K") : tr("XMODEM-CRC");
            m_mode = Checksum;
            qWarning() << "XModem: receiver sent NAK, degraded to Checksum mode";
            emit modeDegraded(fromName, tr("XMODEM-Checksum"));
        }
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
}

/** @brief 处理SendingBlock状态: 解析ACK/NAK/CAN响应，推进或重传数据块
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用，错误时用于清空剩余数据 */
void XModemTransfer::handleStateSendingBlock(char ch, int& readIdx)
{
    if (ch == ACK) {
        // 块确认成功，重置重试计数
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_blockNumber++;
        if (m_blockNumber > 255) m_blockNumber = 1;

        // ACK确认后推进已发送字节数（sendBlock不再自动推进，防止NAK重传时跳块）
        int bs = blockSize();
        qint64 remaining = m_data.size() - m_bytesSent;
        m_bytesSent += qMin(static_cast<qint64>(bs), remaining);

        // 更新速率统计
        updateTransferStats();

        int percent = static_cast<int>((static_cast<qint64>(m_bytesSent) * 100) / m_data.size());
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
        if (m_blockRetryCount > kMaxBlockRetries) {
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
}

/** @brief 处理SendingEOT状态: 解析EOT阶段的ACK/NAK/CAN响应
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void XModemTransfer::handleStateSendingEOT(char ch, int& readIdx)
{
    if (ch == ACK) {
        m_timeoutTimer->stop();
        emit progress(100, m_data.size(), m_data.size());
        m_xmodemState = State::Done;
        finishTransfer();
    } else if (ch == NAK) {
        m_timeoutTimer->stop();
        m_blockRetryCount++;
        if (m_blockRetryCount > kMaxBlockRetries) {
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
}

/** @brief 超时处理，根据当前状态重发数据块或EOT，单块最多重试10次 */
void XModemTransfer::handleTimeout()
{
    if (m_xmodemState == State::SendingBlock) {
        m_blockRetryCount++;
        if (m_blockRetryCount > kMaxBlockRetries) {
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
        if (m_blockRetryCount > kMaxBlockRetries) {
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
        m_timeoutTimer->start(m_timeoutMs * kStartTimeoutMultiplier);
    }
}
