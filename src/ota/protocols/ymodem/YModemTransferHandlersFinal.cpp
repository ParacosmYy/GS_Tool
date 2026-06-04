/**
 * @file YModemTransferHandlersFinal.cpp
 * @brief YMODEM协议后三个状态处理器 - WaitBlock0Ack/WaitFinalC/SendingFinalBlock0
 *
 * 从YModemTransferHandlers.cpp拆分，包含传输后半段(批量切换/会话结束)的状态处理。
 * 传输前半段(WaitingStart/SendingBlock0/SendingData/SendingEOT)见
 * YModemTransferHandlersEarly.cpp。
 */
#include "ota/protocols/ymodem/YModemTransfer.h"

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
        if (m_blockRetryCount > kMaxBlockRetries) {
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
        ++m_totalCancels;  ///< 统计: 最终握手阶段接收方CAN取消
        emit transferError(tr("最终握手阶段被取消"));
        m_receiveBuffer.remove(0, readIdx);
        return;
    }
}
