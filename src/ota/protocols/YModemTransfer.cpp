/**
 * @file YModemTransfer.cpp
 * @brief YMODEM协议传输器实现
 * 增强特性: Block 0文件信息(文件名+大小+修改日期)、批量传输、速率/ETA计算
 */
#include "ota/protocols/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

YModemTransfer::YModemTransfer(QObject* parent) : BaseTransfer(parent) {}

void YModemTransfer::setFilePath(const QString& path) { m_filePaths = QStringList{path}; }
void YModemTransfer::setFilePaths(const QStringList& paths) { m_filePaths = paths; }
double YModemTransfer::transferRate() const { return m_currentRate; }

double YModemTransfer::etaSeconds() const
{
    if (m_currentRate <= 0.0 || m_totalBytes <= 0) return -1.0;
    qint64 remaining = m_totalBytes - m_totalBytesSent;
    if (remaining <= 0) return 0.0;
    return static_cast<double>(remaining) / m_currentRate;
}

// ---- BaseTransfer钩子实现 ----

bool YModemTransfer::onStartInit()
{
    if (m_filePaths.isEmpty()) {
        emit transferError("No files to transfer");
        return false;
    }

    // 计算总字节数
    static constexpr qint64 kMaxFileSize = 1024 * 1024; // 1MB
    m_totalBytes = 0;
    for (const QString& path : m_filePaths) {
        qint64 sz = QFileInfo(path).size();
        if (sz > kMaxFileSize) {
            qWarning() << "YModem: file too large:" << path << sz
                       << "bytes (max" << kMaxFileSize << "bytes)";
            emit transferError(QString("File too large: %1 (%2 bytes, max %3 bytes)")
                                   .arg(path).arg(sz).arg(kMaxFileSize));
            return false;
        }
        m_totalBytes += sz;
    }
    if (m_totalBytes == 0) {
        emit transferError("All files are empty");
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

void YModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

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
        emit transferError("Timeout: retries exceeded (10)");
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
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_ymodemState = State::SendingBlock0;
                sendBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingBlock0:
            if (ch == ACK) {
                // Block 0被接受，转入数据传输阶段
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_ymodemState = State::SendingData;
                m_blockNumber = 1;
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
                    emit transferError("Block 0 rejected: too many retries");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                markError();
                emit transferError("Transfer cancelled by receiver");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingData:
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
                    emit transferError("Too many NAK retries");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                markError();
                emit transferError("Transfer cancelled by receiver");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingEOT:
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
                    emit transferError("EOT acknowledgment failed");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendEOT();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                markError();
                emit transferError("Transfer cancelled during EOT");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::WaitBlock0Ack:
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
            break;

        case State::WaitFinalC:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockRetryCount = 0;
                m_ymodemState = State::SendingFinalBlock0;
                sendFinalBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingFinalBlock0:
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
                    emit transferError("Final Block 0 rejected");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendFinalBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                markError();
                emit transferError("Cancelled during final handshake");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;
        }
    }

    // 单次O(n)压缩
    m_receiveBuffer.remove(0, readIdx);
}

// ---- 数据发送 ----

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

void YModemTransfer::sendEOT()
{
    if (m_conn) {
        m_conn->write(QByteArray(1, EOT));
    }
}

void YModemTransfer::sendFinalBlock0()
{
    // 空Block 0表示传输会话结束
    QByteArray emptyBlock0(kBlockSize, 0x00);
    QByteArray packet = buildBlock(0, emptyBlock0);
    if (m_conn) {
        m_conn->write(packet);
    }
}

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

// ---- 速率统计 ----

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

// ---- 文件管理 ----

bool YModemTransfer::loadNextFile()
{
    if (m_fileIndex >= m_filePaths.size()) {
        emit transferError("No more files to transfer");
        return false;
    }

    QFile file(m_filePaths[m_fileIndex]);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(
            QString("Cannot open file: %1").arg(m_filePaths[m_fileIndex]));
        return false;
    }
    m_currentData = file.readAll();
    file.close();
    m_currentFileName = QFileInfo(m_filePaths[m_fileIndex]).fileName();
    m_bytesSent = 0;

    return true;
}

void YModemTransfer::setState(State s)
{
    m_ymodemState = s;
}
