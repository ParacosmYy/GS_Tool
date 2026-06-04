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

// ── 统计计数器 Getter/Reset ──

/** @brief 获取已发送数据块总数(Block0+数据块) @return 累计块数 */
quint64 YModemTransfer::totalBlocksSent() const { return m_totalBlocksSent; }

/** @brief 获取传输重试总次数(超时/NAK触发的重发) @return 累计重试次数 */
quint64 YModemTransfer::totalRetries() const { return m_totalRetries; }

/** @brief 获取传输错误总次数(CAN取消/写入失败等) @return 累计错误次数 */
quint64 YModemTransfer::totalErrorCount() const { return m_totalErrorCount; }

/** @brief 重置YMODEM传输统计计数器(不影响传输状态) */
void YModemTransfer::resetYmodemStatistics()
{
    m_totalBlocksSent = 0;
    m_totalRetries = 0;
    m_totalErrorCount = 0;
}

/** @brief 设置单个文件路径用于传输
 *  @param path 文件绝对路径 */
void YModemTransfer::setFilePath(const QString& path) { m_filePaths = QStringList{path}; }
/** @brief 设置多个文件路径用于批量传输
 *  @param paths 文件路径列表 */
void YModemTransfer::setFilePaths(const QStringList& paths) { m_filePaths = paths; }
/** @brief 获取当前传输速率
 *  @return 传输速率，单位: 字节/秒 */
double YModemTransfer::transferRate() const { return m_currentRate; }

/** @brief 计算剩余传输时间
 *  @return 预计剩余秒数，无法计算时返回-1 */
double YModemTransfer::etaSeconds() const
{
    if (m_currentRate <= 0.0 || m_totalBytes <= 0) return -1.0;
    qint64 remaining = m_totalBytes - m_totalBytesSent;
    if (remaining <= 0) return 0.0;
    return static_cast<double>(remaining) / m_currentRate;
}

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

/** @brief 更新传输速率统计并发射transferStats信号 */
void YModemTransfer::updateTransferStats()
{
    qint64 elapsedMs = m_transferTimer.elapsed();
    if (elapsedMs <= 0) return;
    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;
    if (elapsedSec > 0.0) {
        m_currentRate = static_cast<double>(m_totalBytesSent) / elapsedSec;
    }
    double eta = etaSeconds();
    emit transferStats(m_currentRate, eta, m_currentFileName);
}

/** @brief 加载下一个待传输文件到内存
 *  @return 文件加载成功返回true，无更多文件或读取失败返回false */
bool YModemTransfer::loadNextFile()
{
    if (m_fileIndex >= m_filePaths.size()) {
        emit transferError(tr("无更多文件可传输"));
        return false;
    }
    QFile file(m_filePaths[m_fileIndex]);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(
            tr("无法打开文件: %1").arg(m_filePaths[m_fileIndex]));
        return false;
    }
    m_currentData = file.readAll();
    if (m_currentData.size() != file.size()) {
        emit transferError(tr("读取文件失败"));
        return false;
    }
    file.close();
    m_currentFileName = QFileInfo(m_filePaths[m_fileIndex]).fileName();
    m_bytesSent = 0;
    return true;
}
