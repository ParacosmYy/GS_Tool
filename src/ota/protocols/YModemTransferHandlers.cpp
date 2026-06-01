/**
 * @file YModemTransferHandlers.cpp
 * @brief YMODEM协议状态处理器 - 协议状态机的各状态处理方法
 *
 * 从YModemTransfer.cpp拆分，包含7个状态处理方法。
 * 由processReceivedData()逐字节解析后根据协议状态分发调用。
 */
#include "ota/protocols/YModemTransfer.h"
#include "utils/CRC.h"

/** @brief 处理WaitingStart状态: 解析C/NAK启动信号，开始发送Block 0
 *  @param ch 接收到的单字节 */
void YModemTransfer::handleStateWaitingStart(char ch)
{
    if (ch == CRC_CHAR || ch == NAK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_ymodemState = State::SendingBlock0;
        sendBlock0();
        m_timeoutTimer->start(m_timeoutMs);
    }
}

/** @brief 处理SendingBlock0状态: 解析Block 0阶段的ACK/C/NAK/CAN响应
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void YModemTransfer::handleStateSendingBlock0(char ch, int& readIdx)
{
    if (ch == ACK) {
        // Block 0被接受 — 不要立即转换到SendingData
        // 标准YMODEM流程: ACK之后接收方还会发一个'C'表示准备接收数据
        // 等待'C'由下方的CRC_CHAR分支处理(调用sendBlock并启动定时器)
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_blockNumber = 1;
        // 重启超时定时器，防止接收方ACK后不发'C'导致无限挂起
        m_timeoutTimer->start(m_timeoutMs * 3);
    } else if (ch == CRC_CHAR) {
        // 接收方ACK后立即发C，开始数据传输
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_ymodemState = State::SendingData;
        m_blockNumber = 1;
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (ch == NAK) {
        // Block 0被拒绝，重发
        m_timeoutTimer->stop();
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            sendCancelBytes();
            m_ymodemState = State::Error;
            markError();
            emit transferError(tr("Block 0 被拒绝: 重试次数过多"));
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
        sendBlock0();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (ch == CAN) {
        m_timeoutTimer->stop();
        m_ymodemState = State::Error;
        markError();
        emit transferError(tr("接收方取消传输"));
        m_receiveBuffer.remove(0, readIdx);
        return;
    }
}

/** @brief 处理SendingData状态: 解析数据传输阶段的ACK/NAK/CAN响应
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void YModemTransfer::handleStateSendingData(char ch, int& readIdx)
{
    if (ch == ACK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_blockNumber++;
        // 更新速率统计
        updateTransferStats();
        int percent = static_cast<int>(
            (m_totalBytesSent * 100) / m_totalBytes);
        emit progress(percent, m_totalBytesSent, m_totalBytes);
        if (m_bytesSent >= m_currentData.size()) {
            m_ymodemState = State::SendingEOT;
            m_blockRetryCount = 0;
            sendEOT();
            m_timeoutTimer->start(m_timeoutMs);
        } else {
            sendBlock();
            m_timeoutTimer->start(m_timeoutMs);
        }
    } else if (ch == NAK) {
        m_timeoutTimer->stop();
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            sendCancelBytes();
            m_ymodemState = State::Error;
            markError();
            emit transferError(tr("NAK重试次数过多"));
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (ch == CAN) {
        m_timeoutTimer->stop();
        m_ymodemState = State::Error;
        markError();
        emit transferError(tr("接收方取消传输"));
        m_receiveBuffer.remove(0, readIdx);
        return;
    }
}

/** @brief 处理SendingEOT状态: 解析EOT阶段的ACK/NAK/CAN，管理批量文件切换
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void YModemTransfer::handleStateSendingEOT(char ch, int& readIdx)
{
    if (ch == ACK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;

        // 发射单文件完成信号
        emit fileTransferComplete(m_currentFileName, m_fileIndex);

        m_fileIndex++;
        if (m_fileIndex < m_filePaths.size()) {
            // 批量传输: 等待下一个文件的C/NAK
            m_ymodemState = State::WaitBlock0Ack;
            m_timeoutTimer->start(m_timeoutMs * 3);
        } else {
            // 所有文件传输完成: 等待最终C发送空Block 0
            m_ymodemState = State::WaitFinalC;
            m_timeoutTimer->start(m_timeoutMs * 3);
        }
    } else if (ch == NAK) {
        m_timeoutTimer->stop();
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            sendCancelBytes();
            m_ymodemState = State::Error;
            markError();
            emit transferError(tr("EOT确认失败"));
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (ch == CAN) {
        m_timeoutTimer->stop();
        m_ymodemState = State::Error;
        markError();
        emit transferError(tr("EOT阶段传输被取消"));
        m_receiveBuffer.remove(0, readIdx);
        return;
    }
}

/** @brief 处理WaitBlock0Ack状态: 等待下一个文件的C/NAK信号后加载并发送Block 0
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void YModemTransfer::handleStateWaitBlock0Ack(char ch, int& readIdx)
{
    if (ch == CRC_CHAR || ch == NAK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        // 加载下一个文件
        if (!loadNextFile()) {
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
        m_ymodemState = State::SendingBlock0;
        sendBlock0();
        m_timeoutTimer->start(m_timeoutMs);
    }
}

/** @brief 处理WaitFinalC状态: 等待最终C/NAK信号后发送空Block 0结束会话
 *  @param ch 接收到的单字节 */
void YModemTransfer::handleStateWaitFinalC(char ch)
{
    if (ch == CRC_CHAR || ch == NAK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_blockRetryCount = 0;
        m_ymodemState = State::SendingFinalBlock0;
        sendFinalBlock0();
        m_timeoutTimer->start(m_timeoutMs);
    }
}

/** @brief 处理SendingFinalBlock0状态: 解析最终空Block 0的ACK/NAK/CAN响应
 *  @param ch 接收到的单字节
 *  @param readIdx 缓冲区读取索引引用 */
void YModemTransfer::handleStateSendingFinalBlock0(char ch, int& readIdx)
{
    if (ch == ACK) {
        m_timeoutTimer->stop();
        emit progress(100, m_totalBytes, m_totalBytes);
        m_ymodemState = State::Done;
        finishTransfer();
    } else if (ch == NAK) {
        m_timeoutTimer->stop();
        m_blockRetryCount++;
        if (m_blockRetryCount > 10) {
            sendCancelBytes();
            m_ymodemState = State::Error;
            markError();
            emit transferError(tr("最终 Block 0 被拒绝"));
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
        sendFinalBlock0();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (ch == CAN) {
        m_timeoutTimer->stop();
        m_ymodemState = State::Error;
        markError();
        emit transferError(tr("最终握手阶段被取消"));
        m_receiveBuffer.remove(0, readIdx);
        return;
    }
}
