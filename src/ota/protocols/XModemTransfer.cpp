#include "ota/protocols/XModemTransfer.h"
#include <QFile>

XModemTransfer::XModemTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &XModemTransfer::onTimeout);
}

void XModemTransfer::setConnection(IConnection* conn)
{
    if (m_conn) {
        disconnect(m_conn, nullptr, this, nullptr);
    }
    m_conn = conn;
    if (m_conn) {
        connect(m_conn, &IConnection::dataReceived,
                this, &XModemTransfer::onConnectionReadyRead);
    }
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

bool XModemTransfer::start()
{
    if (m_state != State::Idle) return false;
    if (!m_conn) {
        emit transferError("No connection set");
        return false;
    }

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
    m_retryCount = 0;
    m_cancelled = false;
    m_receiveBuffer.clear();

    // 等待接收方发送启动信号(NAK=Checksum模式, 'C'=CRC模式)
    setState(State::WaitingForStart);
    m_timeoutTimer->start(kTimeoutMs * 3); // 启动等待超时较长
    return true;
}

void XModemTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && m_state != State::Idle) {
        // 发送两次CAN表示取消
        m_conn->write(QByteArray(2, CAN));
    }
    m_timeoutTimer->stop();
    setState(State::Idle);
    emit transferError("Transfer cancelled by user");
}

bool XModemTransfer::isRunning() const
{
    return m_state != State::Idle && m_state != State::Done && m_state != State::Error;
}

void XModemTransfer::onConnectionReadyRead(const QByteArray& data)
{
    m_receiveBuffer.append(data);
    processReceivedData();
}

void XModemTransfer::onTimeout()
{
    if (m_state == State::Idle) return;

    m_retryCount++;
    if (m_retryCount > kMaxRetries) {
        if (m_conn) m_conn->write(QByteArray(2, CAN));
        setState(State::Error);
        emit transferError("Transfer timeout: max retries exceeded");
        return;
    }

    // 超时重发当前块
    if (m_state == State::SendingBlock) {
        sendBlock();
        m_timeoutTimer->start(kTimeoutMs);
    } else if (m_state == State::SendingEOT) {
        sendEOT();
        m_timeoutTimer->start(kTimeoutMs);
    } else if (m_state == State::WaitingForStart) {
        m_timeoutTimer->start(kTimeoutMs * 3);
    }
}

void XModemTransfer::processReceivedData()
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

        case State::WaitingForStart:
            if (ch == NAK) {
                // 接收方请求Checksum模式
                if (m_mode == Checksum) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    setState(State::SendingBlock);
                    sendBlock();
                    m_timeoutTimer->start(kTimeoutMs);
                } else {
                    // 我们想用CRC/1K模式，但对方只支持Checksum
                    // 降级到Checksum模式
                    m_mode = Checksum;
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    setState(State::SendingBlock);
                    sendBlock();
                    m_timeoutTimer->start(kTimeoutMs);
                }
            } else if (ch == CRC_CHAR) {
                // 接收方请求CRC模式
                m_timeoutTimer->stop();
                m_retryCount = 0;
                setState(State::SendingBlock);
                sendBlock();
                m_timeoutTimer->start(kTimeoutMs);
            }
            break;

        case State::SendingBlock:
            if (ch == ACK) {
                m_timeoutTimer->stop();
                m_retryCount = 0;
                m_blockNumber++;
                // 更新进度
                int percent = static_cast<int>((m_bytesSent * 100) / m_data.size());
                emit progress(percent, m_bytesSent, m_data.size());

                if (m_bytesSent >= m_data.size()) {
                    // 所有数据发送完毕，发EOT
                    setState(State::SendingEOT);
                    sendEOT();
                    m_timeoutTimer->start(kTimeoutMs);
                } else {
                    sendBlock();
                    m_timeoutTimer->start(kTimeoutMs);
                }
            } else if (ch == NAK) {
                // 重发当前块
                m_timeoutTimer->stop();
                m_retryCount++;
                if (m_retryCount > kMaxRetries) {
                    if (m_conn) m_conn->write(QByteArray(2, CAN));
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
                finishTransfer();
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
        }
    }
}

void XModemTransfer::sendBlock()
{
    int bs = blockSize();
    qint64 offset = static_cast<qint64>(m_blockNumber - 1) * bs;
    int dataSize = qMin(static_cast<int>(m_data.size() - offset), bs);

    if (dataSize <= 0) {
        // 没有更多数据，发EOT
        setState(State::SendingEOT);
        sendEOT();
        m_timeoutTimer->start(kTimeoutMs);
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

void XModemTransfer::finishTransfer()
{
    setState(State::Done);
    emit progress(100, m_data.size(), m_data.size());
    emit transferComplete();
}

QByteArray XModemTransfer::buildBlock(int blockNum, const QByteArray& blockData)
{
    QByteArray packet;

    // 头部: SOH(128B)或STX(1024B)
    packet.append((m_mode == OneK) ? STX : SOH);

    // 块号: blockNum(1-255循环)
    char bn = static_cast<char>(blockNum & 0xFF);
    packet.append(bn);
    packet.append(static_cast<char>(~bn & 0xFF)); // 块号反码

    // 数据
    packet.append(blockData);

    // 校验
    if (m_mode == Checksum) {
        // Sum8: 所有字节之和的低8位
        packet.append(static_cast<char>(CRC::checksum(blockData)));
    } else {
        // CRC16 (CRC和1K模式都用CRC16)
        quint16 crc = xmodemCrc(blockData);
        packet.append(static_cast<char>((crc >> 8) & 0xFF));
        packet.append(static_cast<char>(crc & 0xFF));
    }

    return packet;
}

quint16 XModemTransfer::xmodemCrc(const QByteArray& data)
{
    // XMODEM使用的CRC16-CCITT
    return CRC::crc16Ccitt(data);
}

void XModemTransfer::setState(State newState)
{
    m_state = newState;
}
