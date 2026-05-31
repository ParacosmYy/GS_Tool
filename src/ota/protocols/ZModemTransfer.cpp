#include "ota/protocols/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>

ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;  // ZMODEM使用更长的超时
}

void ZModemTransfer::setFilePath(const QString& path)
{
    m_filePath = path;
}

bool ZModemTransfer::onStartInit()
{
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

    // 发送ZRQINIT请求接收方初始化
    m_zmodemState = State::WaitingRinit;
    sendZRQINIT();
    m_timeoutTimer->start(m_timeoutMs);
    return true;
}

void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));          // 8次backspace
        m_conn->write(QByteArray(2, static_cast<char>(0x18))); // 2次CAN
    }
}

void ZModemTransfer::handleTimeout()
{
    switch (m_zmodemState) {
    case State::WaitingRinit:
        sendZRQINIT();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFile:
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::WaitingZAck:
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFin:
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
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
            switch (m_zmodemState) {
            case State::Idle:
            case State::Done:
            case State::Error:
                return;

            case State::WaitingRinit:
                if (type == ZRINIT) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    m_zmodemState = State::SendingFile;
                    sendZFILE();
                    m_timeoutTimer->start(m_timeoutMs);
                }
                break;

            case State::SendingFile:
                if (type == ZRPOS) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    if (headerData.size() >= 4) {
                        m_fileOffset = 0;
                        for (int i = 3; i >= 0; --i) {
                            m_fileOffset = (m_fileOffset << 8) |
                                           (static_cast<quint8>(headerData[i]));
                        }
                    }
                    m_bytesSent = m_fileOffset;
                    m_zmodemState = State::SendingData;
                    sendDataSubpackets();
                } else if (type == ZSKIP) {
                    m_timeoutTimer->stop();
                    m_zmodemState = State::SendingFin;
                    sendZFIN();
                    m_timeoutTimer->start(m_timeoutMs);
                } else if (type == ZRINIT) {
                    m_timeoutTimer->stop();
                    sendZFILE();
                    m_timeoutTimer->start(m_timeoutMs);
                }
                break;

            case State::SendingData:
                if (type == ZRPOS) {
                    m_timeoutTimer->stop();
                    m_retryCount++;
                    if (m_retryCount > m_maxRetries) {
                        sendCancelBytes();
                        m_zmodemState = State::Error;
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
                    m_zmodemState = State::SendingEof;
                    sendZEOF();
                    m_timeoutTimer->start(m_timeoutMs);
                }
                break;

            case State::SendingEof:
                if (type == ZRINIT) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    m_zmodemState = State::SendingFin;
                    sendZFIN();
                    m_timeoutTimer->start(m_timeoutMs);
                } else if (type == ZSKIP) {
                    m_timeoutTimer->stop();
                    m_zmodemState = State::SendingFin;
                    sendZFIN();
                    m_timeoutTimer->start(m_timeoutMs);
                }
                break;

            case State::SendingFin:
                if (type == ZFIN) {
                    m_timeoutTimer->stop();
                    if (m_conn) {
                        m_conn->write(QByteArray("OO"));
                    }
                    emit progress(100, m_fileData.size(), m_fileData.size());
                    finishTransfer();
                }
                break;
            }
        } else {
            // 数据不足解析完整帧，等待更多数据
            if (m_receiveBuffer.size() > 4096) {
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

bool ZModemTransfer::parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData)
{
    // ZMODEM HEX帧格式: ZPAD ZDLE ZHEX type f1 f2 f3 f4 crc1 crc2 [CR] [LF]
    if (data.size() < 7) return false;

    int idx = 0;
    if (data[idx] != ZPAD) return false;
    idx++;

    if (idx >= data.size() || static_cast<quint8>(data[idx]) != ZDLE) return false;
    idx++;

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

    int typeVal = readHexByte(idx);
    if (typeVal < 0) return false;
    type = typeVal;

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
        if (idx < data.size()) idx++;
    }

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
    for (int i = 0; i < 4; ++i) {
        payload.append(i < data.size() ? data[i] : '\0');
    }

    quint16 crc = CRC::crc16Ccitt(payload);
    payload.append(static_cast<char>((crc >> 8) & 0xFF));
    payload.append(static_cast<char>(crc & 0xFF));

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

    quint32 crc = CRC::crc32(payload);
    payload.append(static_cast<char>((crc >> 24) & 0xFF));
    payload.append(static_cast<char>((crc >> 16) & 0xFF));
    payload.append(static_cast<char>((crc >> 8) & 0xFF));
    payload.append(static_cast<char>(crc & 0xFF));

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

    for (char b : data) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 || c == 0x2A) {
            packet.append(ZDLE);
            packet.append(static_cast<char>(c ^ 0x40));
        } else {
            packet.append(b);
        }
    }

    QByteArray crcInput = data;
    crcInput.append(endFlag);
    quint32 crc = CRC::crc32(crcInput);

    packet.append(ZDLE);
    packet.append(endFlag);

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

    QByteArray header = buildBinHeader(ZFILE);

    QFileInfo info(m_filePath);
    QString fileInfo = QString("%1 %2 0")
        .arg(info.fileName())
        .arg(info.size());
    QByteArray fileInfoData = fileInfo.toUtf8();
    fileInfoData.append('\0');

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

    qint64 offset = m_fileOffset;
    while (offset < m_fileData.size()) {
        int chunkSize = qMin(static_cast<int>(m_fileData.size() - offset), kDataLen);
        QByteArray chunk = m_fileData.mid(offset, chunkSize);

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
        m_zmodemState = State::WaitingZAck;
        m_timeoutTimer->start(m_timeoutMs);
    }
}

void ZModemTransfer::sendZEOF()
{
    if (!m_conn) return;

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
    m_zmodemState = s;
}
