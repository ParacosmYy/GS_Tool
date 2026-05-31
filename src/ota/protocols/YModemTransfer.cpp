#include "ota/protocols/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

YModemTransfer::YModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
}

void YModemTransfer::setFilePath(const QString& path)
{
    m_filePaths = QStringList{path};
}

void YModemTransfer::setFilePaths(const QStringList& paths)
{
    m_filePaths = paths;
}

bool YModemTransfer::onStartInit()
{
    if (m_filePaths.isEmpty()) {
        emit transferError("No files to transfer");
        return false;
    }

    // 计算总字节数
    m_totalBytes = 0;
    for (const QString& path : m_filePaths) {
        m_totalBytes += QFileInfo(path).size();
    }
    if (m_totalBytes == 0) {
        emit transferError("All files are empty");
        return false;
    }

    m_fileIndex = 0;
    m_bytesSent = 0;

    // 加载第一个文件
    QFile file(m_filePaths[0]);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(QString("Cannot open file: %1").arg(m_filePaths[0]));
        return false;
    }
    m_currentData = file.readAll();
    file.close();
    m_currentFileName = QFileInfo(m_filePaths[0]).fileName();

    // YMODEM启动: 等待接收方发送'C'(CRC模式)
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
    // 超时重发当前状态
    switch (m_ymodemState) {
    case State::WaitingStart:
        m_timeoutTimer->start(m_timeoutMs * 3);
        break;
    case State::SendingBlock0:
        sendBlock0();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingData:
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingEOT:
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::WaitBlock0Ack:
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::WaitFinalC:
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFinalBlock0:
        sendFinalBlock0();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    default:
        break;
    }
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
                m_ymodemState = State::SendingBlock0;
                sendBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingBlock0:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_ymodemState = State::SendingData;
                m_blockNumber = 1;
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > m_maxRetries) {
                    sendCancelBytes();
                    m_ymodemState = State::Error;
                    emit transferError("Block 0 rejected: too many retries");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                emit transferError("Transfer cancelled by receiver");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingData:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockNumber++;

                int percent = static_cast<int>((m_bytesSent * 100) / m_totalBytes);
                emit progress(percent, m_bytesSent, m_totalBytes);

                if (m_bytesSent >= m_currentData.size()) {
                    m_ymodemState = State::SendingEOT;
                    sendEOT();
                    m_timeoutTimer->start(m_timeoutMs);
                } else {
                    sendBlock();
                    m_timeoutTimer->start(m_timeoutMs);
                }
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > m_maxRetries) {
                    sendCancelBytes();
                    m_ymodemState = State::Error;
                    emit transferError("Too many NAK retries");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_ymodemState = State::Error;
                emit transferError("Transfer cancelled by receiver");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingEOT:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_fileIndex++;

                if (m_fileIndex < m_filePaths.size()) {
                    m_ymodemState = State::WaitBlock0Ack;
                    m_timeoutTimer->start(m_timeoutMs);
                } else {
                    m_ymodemState = State::WaitFinalC;
                    m_timeoutTimer->start(m_timeoutMs);
                }
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > m_maxRetries) {
                    m_ymodemState = State::Error;
                    emit transferError("EOT acknowledgment failed");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendEOT();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::WaitBlock0Ack:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;

                QFile file(m_filePaths[m_fileIndex]);
                if (!file.open(QIODevice::ReadOnly)) {
                    sendCancelBytes();
                    m_ymodemState = State::Error;
                    emit transferError(QString("Cannot open file: %1").arg(m_filePaths[m_fileIndex]));
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                m_currentData = file.readAll();
                file.close();
                m_currentFileName = QFileInfo(m_filePaths[m_fileIndex]).fileName();
                m_bytesSent = 0;

                m_ymodemState = State::SendingBlock0;
                sendBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::WaitFinalC:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_ymodemState = State::SendingFinalBlock0;
                sendFinalBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingFinalBlock0:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                emit progress(100, m_totalBytes, m_totalBytes);
                finishTransfer();
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > m_maxRetries) {
                    sendCancelBytes();
                    m_ymodemState = State::Error;
                    emit transferError("Final Block 0 rejected");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendFinalBlock0();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;
        }
    }

    // 单次O(n)压缩，替代循环中每次O(n)的remove
    m_receiveBuffer.remove(0, readIdx);
}

void YModemTransfer::sendBlock0()
{
    QByteArray block0 = buildBlock0(m_currentFileName, m_currentData.size());
    QByteArray packet = buildBlock(0, block0);
    if (m_conn) {
        m_conn->write(packet);
    }
}

void YModemTransfer::sendBlock()
{
    qint64 offset = static_cast<qint64>(m_blockNumber - 1) * kBlockSize;
    int dataSize = qMin(static_cast<int>(m_currentData.size() - offset), kBlockSize);

    if (dataSize <= 0) {
        m_ymodemState = State::SendingEOT;
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
    m_bytesSent = qMin(offset + dataSize, static_cast<qint64>(m_currentData.size()));
}

void YModemTransfer::sendEOT()
{
    if (m_conn) {
        m_conn->write(QByteArray(1, EOT));
    }
}

void YModemTransfer::sendFinalBlock0()
{
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

    quint16 crc = CRC::crc16Ccitt(blockData);
    packet.append(static_cast<char>((crc >> 8) & 0xFF));
    packet.append(static_cast<char>(crc & 0xFF));

    return packet;
}

QByteArray YModemTransfer::buildBlock0(const QString& fileName, qint64 fileSize)
{
    QByteArray block0;

    // 文件名(ASCII, null-terminated)
    block0.append(fileName.toUtf8());
    block0.append('\0');

    // 文件大小(ASCII十进制, null-terminated)
    block0.append(QString::number(fileSize).toUtf8());
    block0.append('\0');

    // 修改时间(Octal, 简化为0)
    block0.append('0');
    block0.append('\0');

    // 文件权限(简化为0)
    block0.append('0');
    block0.append('\0');

    // 填充到128字节
    if (block0.size() < kBlockSize) {
        block0.append(QByteArray(kBlockSize - block0.size(), 0x00));
    } else if (block0.size() > kBlockSize) {
        block0 = block0.left(kBlockSize);
    }

    return block0;
}

void YModemTransfer::setState(State s)
{
    m_ymodemState = s;
}
