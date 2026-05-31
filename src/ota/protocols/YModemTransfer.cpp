#include "ota/protocols/YModemTransfer.h"
#include <QFile>
#include <QFileInfo>

YModemTransfer::YModemTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &YModemTransfer::onTimeout);
}

void YModemTransfer::setConnection(IConnection* conn)
{
    if (m_conn) {
        disconnect(m_conn, nullptr, this, nullptr);
    }
    m_conn = conn;
    if (m_conn) {
        connect(m_conn, &IConnection::dataReceived,
                this, &YModemTransfer::onConnectionReadyRead);
    }
}

void YModemTransfer::setFilePath(const QString& path)
{
    m_filePaths = QStringList{path};
}

void YModemTransfer::setFilePaths(const QStringList& paths)
{
    m_filePaths = paths;
}

bool YModemTransfer::start()
{
    if (m_state != State::Idle) return false;
    if (!m_conn) {
        emit transferError("No connection set");
        return false;
    }
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
    m_retryCount = 0;
    m_cancelled = false;
    m_receiveBuffer.clear();

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
    setState(State::WaitingStart);
    m_timeoutTimer->start(kTimeoutMs * 3);
    return true;
}

void YModemTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && m_state != State::Idle) {
        sendCancel();
    }
    m_timeoutTimer->stop();
    setState(State::Idle);
    emit transferError("Transfer cancelled by user");
}

bool YModemTransfer::isRunning() const
{
    return m_state != State::Idle && m_state != State::Done && m_state != State::Error;
}

void YModemTransfer::onConnectionReadyRead(const QByteArray& data)
{
    m_receiveBuffer.append(data);
    processReceivedData();
}

void YModemTransfer::onTimeout()
{
    if (m_state == State::Idle) return;

    m_retryCount++;
    if (m_retryCount > kMaxRetries) {
        sendCancel();
        setState(State::Error);
        emit transferError("Transfer timeout: max retries exceeded");
        return;
    }

    // 超时重发当前状态
    switch (m_state) {
    case State::WaitingStart:
        m_timeoutTimer->start(kTimeoutMs * 3);
        break;
    case State::SendingBlock0:
        sendBlock0();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::SendingData:
        sendBlock();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::SendingEOT:
        sendEOT();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::WaitBlock0Ack:
        // 接收方在Block 0 ACK后应再发'C'请求下一个Block 0
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::WaitFinalC:
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::SendingFinalBlock0:
        sendFinalBlock0();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    default:
        break;
    }
}

void YModemTransfer::processReceivedData()
{
    while (!m_receiveBuffer.isEmpty()) {
        char ch = m_receiveBuffer.at(0);
        m_receiveBuffer.remove(0, 1);

        if (m_cancelled) return;

        switch (m_state) {
        case State::Idle:
        case State::Done:
        case State::Error:
            return;

        case State::WaitingStart:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                // 发送Block 0(文件信息)
                setState(State::SendingBlock0);
                sendBlock0();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;

        case State::SendingBlock0:
            if (ch == ACK) {
                // Block 0被接受，等待接收方再发'C'开始数据传输
                m_timeoutTimer->stop();
                m_retryCount = 0;
                setState(State::SendingData);
                m_blockNumber = 1;
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > kMaxRetries) {
                    sendCancel();
                    setState(State::Error);
                    emit transferError("Block 0 rejected: too many retries");
                    return;
                }
                sendBlock0();
                m_timeoutTimer->start(kTimeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                setState(State::Error);
                emit transferError("Transfer cancelled by receiver");
                return;
            }
            break;

        case State::SendingData:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockNumber++;

                // 更新进度
                int percent = static_cast<int>((m_bytesSent * 100) / m_totalBytes);
                emit progress(percent, m_bytesSent, m_totalBytes);

                if (m_bytesSent >= m_currentData.size()) {
                    // 当前文件传输完毕，发EOT
                    setState(State::SendingEOT);
                    sendEOT();
                    m_timeoutTimer->start(kTimeoutMs);
                } else {
                    sendBlock();
                    m_timeoutTimer->start(kTimeoutMs);
                }
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > kMaxRetries) {
                    sendCancel();
                    setState(State::Error);
                    emit transferError("Too many NAK retries");
                    return;
                }
                sendBlock();
                m_timeoutTimer->start(kTimeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                setState(State::Error);
                emit transferError("Transfer cancelled by receiver");
                return;
            }
            break;

        case State::SendingEOT:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_fileIndex++;

                if (m_fileIndex < m_filePaths.size()) {
                    // 还有更多文件，等待接收方发'C'然后发下一个Block 0
                    setState(State::WaitBlock0Ack);
                    m_timeoutTimer->start(kTimeoutMs);
                } else {
                    // 所有文件传输完毕，等待'C'然后发空Block 0
                    setState(State::WaitFinalC);
                    m_timeoutTimer->start(kTimeoutMs);
                }
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > kMaxRetries) {
                    setState(State::Error);
                    emit transferError("EOT acknowledgment failed");
                    return;
                }
                sendEOT();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;

        case State::WaitBlock0Ack:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;

                // 加载下一个文件
                QFile file(m_filePaths[m_fileIndex]);
                if (!file.open(QIODevice::ReadOnly)) {
                    sendCancel();
                    setState(State::Error);
                    emit transferError(QString("Cannot open file: %1").arg(m_filePaths[m_fileIndex]));
                    return;
                }
                m_currentData = file.readAll();
                file.close();
                m_currentFileName = QFileInfo(m_filePaths[m_fileIndex]).fileName();
                m_bytesSent = 0;

                // 发送下一个文件的Block 0
                setState(State::SendingBlock0);
                sendBlock0();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;

        case State::WaitFinalC:
            if (ch == CRC_CHAR || ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                setState(State::SendingFinalBlock0);
                sendFinalBlock0();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;

        case State::SendingFinalBlock0:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                finishTransfer();
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > kMaxRetries) {
                    sendCancel();
                    setState(State::Error);
                    emit transferError("Final Block 0 rejected");
                    return;
                }
                sendFinalBlock0();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;
        }
    }
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
        // 当前文件数据发完，发EOT
        setState(State::SendingEOT);
        sendEOT();
        m_timeoutTimer->start(kTimeoutMs);
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
    // 空Block 0表示批量传输结束
    QByteArray emptyBlock0(kBlockSize, 0x00);
    QByteArray packet = buildBlock(0, emptyBlock0);
    if (m_conn) {
        m_conn->write(packet);
    }
}

void YModemTransfer::sendCancel()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

void YModemTransfer::finishTransfer()
{
    setState(State::Done);
    emit progress(100, m_totalBytes, m_totalBytes);
    emit transferComplete();
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

    // 修改时间(Octal, 可选, 这里简化为0)
    block0.append('0');
    block0.append('\0');

    // 文件权限(可选, 简化为0)
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
    m_state = s;
}
