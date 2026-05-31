#include "ota/protocols/XModemTransfer.h"
#include <QFile>

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

bool XModemTransfer::onStartInit()
{
    // 加载文件数据
    if (m_data.isEmpty() && !m_filePath.isEmpty()) {
        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit transferError(QString("Cannot open file: %1").arg(m_filePath));
            return false;
        }
        m_data = file.readAll();
        file.close();
    }

    if (m_data.isEmpty()) {
        emit transferError("No data to transfer");
        return false;
    }

    m_blockNumber = 1;
    m_bytesSent = 0;

    // 等待接收方发送启动信号(NAK=Checksum模式, 'C'=CRC模式)
    m_xmodemState = State::WaitingForStart;
    m_timeoutTimer->start(m_timeoutMs * 3);
    return true;
}

void XModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(2, CAN));
    }
}

void XModemTransfer::handleTimeout()
{
    // 超时重发当前状态
    if (m_xmodemState == State::SendingBlock) {
        sendBlock();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::SendingEOT) {
        sendEOT();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (m_xmodemState == State::WaitingForStart) {
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
                // 接收方请求Checksum模式
                if (m_mode != Checksum) {
                    m_mode = Checksum;
                }
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_xmodemState = State::SendingBlock;
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CRC_CHAR) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_xmodemState = State::SendingBlock;
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;

        case State::SendingBlock:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockNumber++;
                if (m_blockNumber > 255) m_blockNumber = 1;

                int percent = static_cast<int>((m_bytesSent * 100) / m_data.size());
                emit progress(percent, m_bytesSent, m_data.size());

                if (m_bytesSent >= m_data.size()) {
                    m_xmodemState = State::SendingEOT;
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
                    if (m_conn) m_conn->write(QByteArray(2, CAN));
                    m_xmodemState = State::Error;
                    emit transferError("Too many NAK retries");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendBlock();
                m_timeoutTimer->start(m_timeoutMs);
            } else if (ch == CAN) {
                m_timeoutTimer->stop();
                m_xmodemState = State::Error;
                emit transferError("Transfer cancelled by receiver");
                m_receiveBuffer.remove(0, readIdx);
                return;
            }
            break;

        case State::SendingEOT:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                emit progress(100, m_data.size(), m_data.size());
                finishTransfer();
            } else if (ch == NAK) {
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > m_maxRetries) {
                    m_xmodemState = State::Error;
                    emit transferError("EOT acknowledgment failed");
                    m_receiveBuffer.remove(0, readIdx);
                    return;
                }
                sendEOT();
                m_timeoutTimer->start(m_timeoutMs);
            }
            break;
        }
    }

    // 单次O(n)压缩，替代循环中每次O(n)的remove
    m_receiveBuffer.remove(0, readIdx);
}

void XModemTransfer::sendBlock()
{
    int bs = blockSize();
    qint64 offset = static_cast<qint64>((m_blockNumber - 1) % 256) * bs;
    int dataSize = qMin(static_cast<int>(m_data.size() - offset), bs);

    if (dataSize <= 0) {
        m_xmodemState = State::SendingEOT;
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
        m_conn->write(packet);
    }
    m_bytesSent = qMin(offset + dataSize, static_cast<qint64>(m_data.size()));
}

void XModemTransfer::sendEOT()
{
    if (m_conn) {
        m_conn->write(QByteArray(1, EOT));
    }
}

QByteArray XModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;

    // 头部: SOH(128B)或STX(1024B)
    packet.append((m_mode == OneK) ? STX : SOH);

    // 块号: blockNum(1-255循环)
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF));

    // 数据
    packet.append(blockData);

    // 校验
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
    return CRC::crc16Ccitt(data);
}

void XModemTransfer::setState(State newState)
{
    m_xmodemState = newState;
}
