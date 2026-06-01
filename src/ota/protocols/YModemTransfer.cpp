/**
 * @file YModemTransfer.cpp
 * @brief YMODEM协议传输器实现
 * 增强特性: Block 0文件信息(文件名+大小+修改日期)、批量传输、速率/ETA计算
 */
#include "ota/protocols/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

/** @brief 构造函数，初始化YMODEM传输器基类 */
YModemTransfer::YModemTransfer(QObject* parent) : BaseTransfer(parent) {}

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
    m_timeoutTimer->start(m_timeoutMs * 3);
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
        m_timeoutTimer->start(m_timeoutMs * 3);
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
    if (m_blockRetryCount > 10) {
        sendCancelBytes();
        m_ymodemState = State::Error;
        markError();
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

/** @brief 构建并发送Block 0(文件信息块)，包含文件名、大小、修改时间和权限 */
void YModemTransfer::sendBlock0()
{
    // 获取文件修改时间
    qint64 modTime = 0;
    if (m_fileIndex < m_filePaths.size()) {
        modTime = QFileInfo(m_filePaths[m_fileIndex])
                      .lastModified().toSecsSinceEpoch();
    }
    QByteArray block0 = buildBlock0(m_currentFileName, m_currentData.size(),
                                    modTime);
    QByteArray packet = buildBlock(0, block0);
    if (m_conn) {
        m_conn->write(packet);
    }
}

/** @brief 构建并发送数据块，不足128字节时用0x1A填充 */
void YModemTransfer::sendBlock()
{
    qint64 offset = static_cast<qint64>(m_blockNumber - 1) * kBlockSize;
    int dataSize = qMin(static_cast<int>(m_currentData.size() - offset),
                        kBlockSize);
    if (dataSize <= 0) {
        m_ymodemState = State::SendingEOT;
        m_blockRetryCount = 0;
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
        return;
    }
    QByteArray blockData = m_currentData.mid(offset, dataSize);
    if (blockData.size() < kBlockSize) {
        blockData.append(QByteArray(kBlockSize - blockData.size(), 0x1A));
    }
    QByteArray packet = buildBlock(m_blockNumber, blockData);
    if (m_conn) {
        m_conn->write(packet);
    }
    qint64 sent = qMin(offset + dataSize,
                        static_cast<qint64>(m_currentData.size()));
    m_totalBytesSent += (sent - m_bytesSent);
    m_bytesSent = sent;
}

/** @brief 发送EOT(End of Transmission)字节通知接收方当前文件传输结束 */
void YModemTransfer::sendEOT()
{
    if (m_conn) {
        m_conn->write(QByteArray(1, EOT));
    }
}

/** @brief 发送最终空Block 0，表示整个YMODEM传输会话结束 */
void YModemTransfer::sendFinalBlock0()
{
    // 空Block 0表示传输会话结束
    QByteArray emptyBlock0(kBlockSize, 0x00);
    QByteArray packet = buildBlock(0, emptyBlock0);
    if (m_conn) {
        m_conn->write(packet);
    }
}

/** @brief 构建完整数据块包(SOH+块号+数据+CRC16)
 *  @param blockNum 块编号(0=文件信息块, 1-255=数据块)
 *  @param blockData 块数据载荷
 *  @return 完整的数据包字节数组 */
QByteArray YModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;
    packet.append(SOH);
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF));
    packet.append(blockData);
    // YMODEM固定使用CRC16-CCITT
    quint16 crc = CRC::crc16Ccitt(blockData);
    packet.append(static_cast<char>((crc >> 8) & 0xFF));
    packet.append(static_cast<char>(crc & 0xFF));
    return packet;
}

/** @brief 构建Block 0文件信息块(文件名+大小+修改时间+权限)
 *  @param fileName 文件名
 *  @param fileSize 文件大小(字节)
 *  @param modTime 文件修改时间(Unix时间戳)
 *  @return 128字节的Block 0数据 */
QByteArray YModemTransfer::buildBlock0(const QString& fileName,
                                        qint64 fileSize, qint64 modTime)
{
    QByteArray block0;
    // 文件名(ASCII, null-terminated)
    block0.append(fileName.toUtf8());
    block0.append('\0');
    // 文件大小(ASCII十进制, null-terminated)
    block0.append(QString::number(fileSize).toUtf8());
    block0.append('\0');
    // 修改时间(Octal格式的Unix时间戳, null-terminated)
    block0.append(QString::number(modTime, 8).toUtf8());
    block0.append('\0');
    // 文件权限(简化为0o100644 = 普通文件, rw-r--r--)
    block0.append("100644");
    block0.append('\0');
    // 填充到128字节(不足用0x00填充，超过截断)
    if (block0.size() < kBlockSize) {
        block0.append(QByteArray(kBlockSize - block0.size(), 0x00));
    } else if (block0.size() > kBlockSize) {
        block0 = block0.left(kBlockSize);
    }
    return block0;
}

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
