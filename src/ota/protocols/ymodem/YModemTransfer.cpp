/**
 * @file YModemTransfer.cpp
 * @brief YMODEM协议传输器 - 核心传输逻辑
 *
 * 包含构造/析构、公共API、生命周期钩子、超时处理、接收状态机分发、
 * 统计更新与文件加载。
 * 数据包构建与发送方法见 YModemTransferProtocol.cpp。
 * 协议状态处理方法见 YModemTransferHandlers.cpp。
 */
#include "ota/protocols/ymodem/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

/** @brief 构造函数，初始化YMODEM传输器基类 */
YModemTransfer::YModemTransfer(QObject* parent) : BaseTransfer(parent) {}

// ── 统计/文件/速率方法见 YModemTransferStats.cpp ──

/** @brief 传输启动初始化，校验文件列表并加载第一个文件
 *  @return 初始化成功返回true，文件为空或读取失败返回false */
bool YModemTransfer::onStartInit()
{
    if (m_filePaths.isEmpty()) {
        emit transferError(tr("无文件可传输"));
        return false;
    }

    // 计算总字节数
    m_totalBytes = 0;
    for (const QString& path : m_filePaths) {
        qint64 sz = QFileInfo(path).size();
        if (sz > BaseTransfer::kMaxFileSize) {
            qWarning() << "YModem: file too large:" << path << sz
                       << "bytes (max" << BaseTransfer::kMaxFileSize << "bytes)";
            emit transferError(tr("文件过大: %1 (%2 字节, 上限 %3 字节)")
                                   .arg(path).arg(sz).arg(BaseTransfer::kMaxFileSize));
            return false;
        }
        m_totalBytes += sz;
    }
    if (m_totalBytes == 0) {
        emit transferError(tr("所有文件均为空"));
        return false;
    }

    m_fileIndex = 0;
    m_bytesSent = 0;
    m_totalBytesSent = 0;
    m_blockRetryCount = 0;
    m_currentRate = 0.0;
    m_transferTimer.start();
    // 加载第一个文件
    if (!loadNextFile()) {
        return false;
    }
    // YMODEM启动: 等待接收方发送C(CRC模式)
    m_ymodemState = State::WaitingStart;
    m_timeoutTimer->start(m_timeoutMs * kStartTimeoutMultiplier);
    return true;
}

/** @brief 安全写入: 检测write返回值，连接断开时立即终止传输
 *  @param data 待写入的数据
 *  @return true=成功写入, false=连接已断开(已设置Error状态) */
bool YModemTransfer::writeChecked(const QByteArray& data)
{
    if (!m_conn) {
        m_ymodemState = State::Error;
        markError();
        ++m_totalErrorCount;
        emit transferError(tr("连接中断: 连接对象无效"));
        return false;
    }
    qint64 written = m_conn->write(data);
    if (written < 0) {
        m_ymodemState = State::Error;
        markError();
        ++m_totalErrorCount;
        emit transferError(
            tr("连接中断: 写入失败, 已传输 %1/%2 字节")
                .arg(m_totalBytesSent)
                .arg(m_totalBytes));
        return false;
    }
    return true;
}

/** @brief 发送CAN取消字节，连续发送2个CAN通知接收方终止传输 */
void YModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

/** @brief 超时处理，根据当前状态重发数据，每个阶段独立重试最多10次 */
void YModemTransfer::handleTimeout()
{
    // 超时重发当前状态，每个阶段独立重试计数(最多10次)
    switch (m_ymodemState) {
    case State::WaitingStart:
        m_timeoutTimer->start(m_timeoutMs * kStartTimeoutMultiplier);
        return;
    case State::WaitBlock0Ack:
    case State::WaitFinalC:
        m_timeoutTimer->start(m_timeoutMs);
        return;
    default:
        break;
    }

    // 有重试上限的状态统一处理
    m_blockRetryCount++;
    ++m_totalRetries;
    if (m_blockRetryCount > kMaxBlockRetries) {
        sendCancelBytes();
        m_ymodemState = State::Error;
        markError();
        ++m_totalErrorCount;
        emit transferError(tr("超时: 重试次数耗尽 (10次)"));
        return;
    }

    switch (m_ymodemState) {
    case State::SendingBlock0: sendBlock0(); break;
    case State::SendingData:   sendBlock(); break;
    case State::SendingEOT:    sendEOT(); break;
    case State::SendingFinalBlock0: sendFinalBlock0(); break;
    default: break;
    }
    m_timeoutTimer->start(m_timeoutMs);
}

/** @brief 处理接收缓冲区数据，按字节逐个分发给对应状态处理方法 */
void YModemTransfer::processReceivedData()
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

        switch (m_ymodemState) {
        case State::Idle:
        case State::Done:
        case State::Error:
            m_receiveBuffer.remove(0, readIdx);
            return;
        case State::WaitingStart:
            handleStateWaitingStart(ch);
            break;
        case State::SendingBlock0:
            handleStateSendingBlock0(ch, readIdx);
            break;
        case State::SendingData:
            handleStateSendingData(ch, readIdx);
            break;
        case State::SendingEOT:
            handleStateSendingEOT(ch, readIdx);
            break;
        case State::WaitBlock0Ack:
            handleStateWaitBlock0Ack(ch, readIdx);
            break;
        case State::WaitFinalC:
            handleStateWaitFinalC(ch);
            break;
        case State::SendingFinalBlock0:
            handleStateSendingFinalBlock0(ch, readIdx);
            break;
        default:
            qWarning() << "YModem: unknown state" << static_cast<int>(m_ymodemState);
            m_receiveBuffer.remove(0, readIdx);
            return;
        }

        // 处理函数可能导致early return, 重新检查
        if (m_ymodemState == State::Error || m_cancelled) {
            m_receiveBuffer.remove(0, readIdx);
            return;
        }
    }

    // 单次O(n)压缩
    m_receiveBuffer.remove(0, readIdx);
}

// ── 数据包构建与发送方法见 YModemTransferProtocol.cpp ──
