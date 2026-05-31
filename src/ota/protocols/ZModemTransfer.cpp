#include "ota/protocols/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>

ZModemTransfer::ZModemTransfer(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ZModemTransfer::onTimeout);
}

void ZModemTransfer::setConnection(IConnection* conn)
{
    if (m_conn) {
        disconnect(m_conn, nullptr, this, nullptr);
    }
    m_conn = conn;
    if (m_conn) {
        connect(m_conn, &IConnection::dataReceived,
                this, &ZModemTransfer::onConnectionReadyRead);
    }
}

void ZModemTransfer::setFilePath(const QString& path)
{
    m_filePath = path;
}

bool ZModemTransfer::start()
{
    if (m_state != State::Idle) return false;
    if (!m_conn) {
        emit transferError("No connection set");
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(QString("Cannot open file: %1").arg(m_filePath));
        return false;
    }
    m_fileData = file.readAll();
    file.close();

    if (m_fileData.isEmpty()) {
        emit transferError("File is empty");
        return false;
    }

    m_bytesSent = 0;
    m_fileOffset = 0;
    m_retryCount = 0;
    m_cancelled = false;
    m_receiveBuffer.clear();

    // 发送ZRQINIT请求接收方初始化
    setState(State::WaitingRinit);
    sendZRQINIT();
    m_timeoutTimer->start(kTimeoutMs);
    return true;
}

void ZModemTransfer::cancel()
{
    m_cancelled = true;
    if (m_conn && m_state != State::Idle) {
        sendCancel();
    }
    m_timeoutTimer->stop();
    setState(State::Idle);
    emit transferError("Transfer cancelled by user");
}

bool ZModemTransfer::isRunning() const
{
    return m_state != State::Idle && m_state != State::Done && m_state != State::Error;
}

void ZModemTransfer::onConnectionReadyRead(const QByteArray& data)
{
    m_receiveBuffer.append(data);
    processReceivedData();
}

void ZModemTransfer::onTimeout()
{
    if (m_state == State::Idle) return;

    m_retryCount++;
    if (m_retryCount > kMaxRetries) {
        sendCancel();
        setState(State::Error);
        emit transferError("Transfer timeout: max retries exceeded");
        return;
    }

    switch (m_state) {
    case State::WaitingRinit:
        sendZRQINIT();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::SendingFile:
        sendZFILE();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::WaitingZAck:
        m_timeoutTimer->start(kTimeoutMs);
        break;
    case State::SendingFin:
        sendZFIN();
        m_timeoutTimer->start(kTimeoutMs);
        break;
    default:
        break;
    }
}

void ZModemTransfer::processReceivedData()
{
    while (!m_receiveBuffer.isEmpty()) {
        if (m_cancelled) return;

        // 查找ZPAD标记定位帧头
        int padIdx = m_receiveBuffer.indexOf(ZPAD);
        if (padIdx < 0) {
            m_receiveBuffer.clear();
            return;
        }

        // 丢弃ZPAD之前的垃圾数据
        if (padIdx > 0) {
            m_receiveBuffer.remove(0, padIdx);
        }

        // 尝试解析帧
        int type = -1;
        QByteArray headerData;

        if (parseHexFrame(m_receiveBuffer, type, headerData)) {
            // 处理解析到的帧
            switch (m_state) {
            case State::Idle:
            case State::Done:
            case State::Error:
                return;

            case State::WaitingRinit:
                if (type == ZRINIT) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    // 接收方就绪，发送文件信息
                    setState(State::SendingFile);
                    sendZFILE();
                    m_timeoutTimer->start(kTimeoutMs);
                }
                break;

            case State::SendingFile:
                if (type == ZRPOS) {
                    // 接收方告诉我们要从哪里开始
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    if (headerData.size() >= 4) {
                        m_fileOffset = 0;
                        // P0-P3是文件偏移(4字节, little-endian)
                        for (int i = 3; i >= 0; --i) {
                            m_fileOffset = (m_fileOffset << 8) |
                                           (static_cast<quint8>(headerData[i]));
                        }
                    }
                    m_bytesSent = m_fileOffset;
                    setState(State::SendingData);
                    sendDataSubpackets();
                } else if (type == ZSKIP) {
                    m_timeoutTimer->stop();
                    setState(State::SendingFin);
                    sendZFIN();
                    m_timeoutTimer->start(kTimeoutMs);
                } else if (type == ZRINIT) {
                    // 再次ZRINIT，重发ZFILE
                    m_timeoutTimer->stop();
                    sendZFILE();
                    m_timeoutTimer->start(kTimeoutMs);
                }
                break;

            case State::SendingData:
                if (type == ZRPOS) {
                    // 需要重传
                    m_timeoutTimer->stop();
                    m_retryCount++;
                    if (m_retryCount > kMaxRetries) {
                        sendCancel();
                        setState(State::Error);
                        emit transferError("Too many retransmission requests");
                        return;
                    }
                    if (headerData.size() >= 4) {
                        m_fileOffset = 0;
                        for (int i = 3; i >= 0; --i) {
                            m_fileOffset = (m_fileOffset << 8) |
                                           (static_cast<quint8>(headerData[i]));
                        }
                    }
                    m_bytesSent = m_fileOffset;
                    sendDataSubpackets();
                } else if (type == ZACK) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                }
                break;

            case State::WaitingZAck:
                if (type == ZACK || type == ZRPOS) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    // 文件发送完毕，发送ZEOF
                    setState(State::SendingEof);
                    sendZEOF();
                    m_timeoutTimer->start(kTimeoutMs);
                }
                break;

            case State::SendingEof:
                if (type == ZRINIT) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    // 文件被接受，发送ZFIN结束会话
                    setState(State::SendingFin);
                    sendZFIN();
                    m_timeoutTimer->start(kTimeoutMs);
                } else if (type == ZSKIP) {
                    m_timeoutTimer->stop();
                    setState(State::SendingFin);
                    sendZFIN();
                    m_timeoutTimer->start(kTimeoutMs);
                }
                break;

            case State::SendingFin:
                if (type == ZFIN) {
                    m_timeoutTimer->stop();
                    // 发送'OO'(Over and Out)
                    if (m_conn) {
                        m_conn->write(QByteArray("OO"));
                    }
                    finishTransfer();
                }
                break;
            }
        } else {
            // 数据不足解析完整帧，等待更多数据
            if (m_receiveBuffer.size() > 4096) {
                // 防止缓冲区无限增长
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

bool ZModemTransfer::parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData)
{
    // ZMODEM HEX帧格式: ZPAD ZDLE ZHEX type f1 f2 f3 f4 crc1 crc2 [CR] [LF]
    // 最少需要: 1(ZPAD) + 1(ZDLE) + 1(ZHEX) + 1(type) + 4(flags) + 2(crc) = 10 hex chars + overhead
    if (data.size() < 7) return false;

    int idx = 0;
    // 跳过ZPAD
    if (data[idx] != ZPAD) return false;
    idx++;

    // ZDLE
    if (idx >= data.size() || static_cast<quint8>(data[idx]) != ZDLE) return false;
    idx++;

    // 帧格式标记
    if (idx >= data.size()) return false;
    char frameType = data[idx];
    if (frameType != ZHEX) return false;
    idx++;

    // 读取hex字符: type(2) + flags(8) + crc(4) = 14 hex chars
    if (idx + 14 > data.size()) return false;

    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    auto readHexByte = [&](int& offset) -> int {
        int hi = hexVal(data[offset]);
        int lo = hexVal(data[offset + 1]);
        if (hi < 0 || lo < 0) return -1;
        offset += 2;
        return (hi << 4) | lo;
    };

    // 读取类型
    int typeVal = readHexByte(idx);
    if (typeVal < 0) return false;
    type = typeVal;

    // 读取4字节flags
    headerData.clear();
    for (int i = 0; i < 4; ++i) {
        int b = readHexByte(idx);
        if (b < 0) return false;
        headerData.append(static_cast<char>(b));
    }

    // 读取CRC(2字节) - 跳过校验
    int crcHi = readHexByte(idx);
    int crcLo = readHexByte(idx);
    if (crcHi < 0 || crcLo < 0) return false;

    // 跳过可能的CR LF
    while (idx < data.size() && (data[idx] == '\r' || data[idx] == '\n')) {
        idx++;
    }

    // 跳过可能的ZDLE + 行结束
    if (idx < data.size() && static_cast<quint8>(data[idx]) == ZDLE) {
        idx++;
        if (idx < data.size()) idx++; // XON或其他
    }

    // 消费已解析的数据
    m_receiveBuffer.remove(0, idx);
    return true;
}

QByteArray ZModemTransfer::buildHexHeader(quint8 frameType, const QByteArray& data)
{
    QByteArray frame;
    frame.append(ZPAD);
    frame.append(ZDLE);
    frame.append(ZHEX);

    QByteArray payload;
    payload.append(static_cast<char>(frameType));
    // 填充flags到4字节
    for (int i = 0; i < 4; ++i) {
        payload.append(i < data.size() ? data[i] : '\0');
    }

    // 计算CRC16
    quint16 crc = CRC::crc16Ccitt(payload);
    payload.append(static_cast<char>((crc >> 8) & 0xFF));
    payload.append(static_cast<char>(crc & 0xFF));

    // 转换为hex ASCII
    for (char b : payload) {
        frame.append(toHex(static_cast<quint8>(b), 2));
    }
    frame.append("\r\n");

    return frame;
}

QByteArray ZModemTransfer::buildBinHeader(quint8 frameType, const QByteArray& data)
{
    QByteArray frame;
    frame.append(ZPAD);
    frame.append(ZDLE);
    frame.append(ZBIN32);

    QByteArray payload;
    payload.append(static_cast<char>(frameType));
    for (int i = 0; i < 4; ++i) {
        payload.append(i < data.size() ? data[i] : '\0');
    }

    // CRC32
    quint32 crc = CRC::crc32(payload);
    payload.append(static_cast<char>((crc >> 24) & 0xFF));
    payload.append(static_cast<char>((crc >> 16) & 0xFF));
    payload.append(static_cast<char>((crc >> 8) & 0xFF));
    payload.append(static_cast<char>(crc & 0xFF));

    // ZDLE编码
    for (char b : payload) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 ||
            c == 0x2A || (c == 0x00 && frame.size() < 8)) {
            frame.append(ZDLE);
            frame.append(static_cast<char>(c ^ 0x40));
        } else {
            frame.append(b);
        }
    }

    return frame;
}

QByteArray ZModemTransfer::buildDataSubpacket(char endFlag, const QByteArray& data)
{
    QByteArray packet;

    // ZDLE编码数据
    for (char b : data) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 || c == 0x2A) {
            packet.append(ZDLE);
            packet.append(static_cast<char>(c ^ 0x40));
        } else {
            packet.append(b);
        }
    }

    // CRC32 over data + endFlag
    QByteArray crcInput = data;
    crcInput.append(endFlag);
    quint32 crc = CRC::crc32(crcInput);

    // ZDLE + endFlag
    packet.append(ZDLE);
    packet.append(endFlag);

    // CRC32 (4 bytes, ZDLE encoded)
    QByteArray crcBytes;
    crcBytes.append(static_cast<char>((crc >> 24) & 0xFF));
    crcBytes.append(static_cast<char>((crc >> 16) & 0xFF));
    crcBytes.append(static_cast<char>((crc >> 8) & 0xFF));
    crcBytes.append(static_cast<char>(crc & 0xFF));

    for (char b : crcBytes) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 || c == 0x2A) {
            packet.append(ZDLE);
            packet.append(static_cast<char>(c ^ 0x40));
        } else {
            packet.append(b);
        }
    }

    return packet;
}

void ZModemTransfer::sendZRQINIT()
{
    if (m_conn) {
        m_conn->write(buildHexHeader(ZRQINIT));
    }
}

void ZModemTransfer::sendZFILE()
{
    if (!m_conn) return;

    // ZFILE帧头
    QByteArray header = buildBinHeader(ZFILE);

    // 文件信息: "filename size mtime 0"
    QFileInfo info(m_filePath);
    QString fileInfo = QString("%1 %2 0")
        .arg(info.fileName())
        .arg(info.size());
    QByteArray fileInfoData = fileInfo.toUtf8();
    fileInfoData.append('\0');

    // 数据子帧(以ZCRCW结束，等待接收方响应)
    QByteArray subpacket = buildDataSubpacket(ZCRCW, fileInfoData);

    m_conn->write(header);
    m_conn->write(subpacket);
}

void ZModemTransfer::sendZDATA()
{
    if (!m_conn) return;
    QByteArray offsetData;
    offsetData.append(static_cast<char>(m_fileOffset & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 8) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 16) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 24) & 0xFF));

    QByteArray header = buildBinHeader(ZDATA, offsetData);
    m_conn->write(header);
}

void ZModemTransfer::sendDataSubpackets()
{
    if (!m_conn) return;

    sendZDATA();

    // 连续发送数据子帧
    qint64 offset = m_fileOffset;
    while (offset < m_fileData.size()) {
        int chunkSize = qMin(static_cast<int>(m_fileData.size() - offset), kDataLen);
        QByteArray chunk = m_fileData.mid(offset, chunkSize);

        // 最后一个块或达到窗口限制时用ZCRCW等待确认
        bool isLast = (offset + chunkSize >= m_fileData.size());
        char endFlag = isLast ? ZCRCW : ZCRCG;

        QByteArray subpacket = buildDataSubpacket(endFlag, chunk);
        m_conn->write(subpacket);

        offset += chunkSize;
        m_bytesSent = offset;

        int percent = static_cast<int>((offset * 100) / m_fileData.size());
        emit progress(percent, offset, m_fileData.size());
    }

    m_fileOffset = offset;
    m_bytesSent = offset;

    if (m_bytesSent >= m_fileData.size()) {
        setState(State::WaitingZAck);
        m_timeoutTimer->start(kTimeoutMs);
    }
}

void ZModemTransfer::sendZEOF()
{
    if (!m_conn) return;

    // ZEOF: 文件偏移量(文件大小表示传输完成)
    QByteArray offsetData;
    qint64 size = m_fileData.size();
    offsetData.append(static_cast<char>(size & 0xFF));
    offsetData.append(static_cast<char>((size >> 8) & 0xFF));
    offsetData.append(static_cast<char>((size >> 16) & 0xFF));
    offsetData.append(static_cast<char>((size >> 24) & 0xFF));

    m_conn->write(buildHexHeader(ZEOF, offsetData));
}

void ZModemTransfer::sendZFIN()
{
    if (m_conn) {
        m_conn->write(buildHexHeader(ZFIN));
    }
}

void ZModemTransfer::sendCancel()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08)); // 8次backspace
        m_conn->write(QByteArray(2, static_cast<char>(0x18))); // 2次CAN
    }
}

void ZModemTransfer::finishTransfer()
{
    setState(State::Done);
    emit progress(100, m_fileData.size(), m_fileData.size());
    emit transferComplete();
}

QByteArray ZModemTransfer::toHex(quint32 val, int digits)
{
    QByteArray result;
    for (int i = digits - 1; i >= 0; --i) {
        int nibble = (val >> (i * 4)) & 0xF;
        result.append(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10));
    }
    return result;
}

void ZModemTransfer::setState(State s)
{
    m_state = s;
}
