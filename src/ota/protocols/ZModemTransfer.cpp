/**
 * @file ZModemTransfer.cpp
 * @brief ZMODEM协议传输器实现
 *
 * 实现ZMODEM Sender端状态机: HEX帧解析(16位CRC校验)、BIN32帧构建、
 * 数据子帧发送(ZCRCG/ZCRCW)、超时重发、断点续传(ZRPOS偏移处理)
 */
#include "ota/protocols/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>

// ---- 构造与配置 ----

ZModemTransfer::ZModemTransfer(QObject* parent) : BaseTransfer(parent) { m_timeoutMs = 10000; }
void ZModemTransfer::setFilePath(const QString& path) { m_filePath = path; }

// ---- BaseTransfer钩子实现 ----

/** @brief 协议初始化: 加载文件并发送ZRQINIT握手帧 */
bool ZModemTransfer::onStartInit()
{
    // 文件大小校验(最大1MB)
    static constexpr qint64 kMaxFileSize = 1024 * 1024;
    QFileInfo fileInfo(m_filePath);
    if (fileInfo.size() > kMaxFileSize) {
        qWarning() << "ZModem: file too large:" << fileInfo.size();
        emit transferError(QString("File too large: %1 (%2 bytes, max %3 bytes)")
                               .arg(m_filePath).arg(fileInfo.size()).arg(kMaxFileSize));
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
    m_senderCrc32 = 0;

    // 发送ZRQINIT请求接收方初始化
    m_zmodemState = State::WaitingRinit;
    sendZRQINIT();
    m_timeoutTimer->start(m_timeoutMs);
    return true;
}

/** @brief 发送取消帧: 8次BS + 2次CAN */
void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));
        m_conn->write(QByteArray(2, static_cast<char>(0x18)));
    }
}

/** @brief 超时处理: 根据当前状态重发对应帧 */
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

/** @brief ZMODEM状态机: 解析HEX帧(CRC16校验)并执行状态转移 */
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

        int type = -1;
        QByteArray headerData;

        if (parseHexFrame(m_receiveBuffer, type, headerData)) {
            switch (m_zmodemState) {
            case State::Idle:
            case State::Done:
            case State::Error:
                return;

            case State::WaitingRinit:
                // 收到ZRINIT: 发送文件信息
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
                    // 断点续传: 解析4字节小端偏移
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    if (headerData.size() >= 4) {
                        m_fileOffset = 0;
                        for (int i = 3; i >= 0; --i)
                            m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
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
                    // 重传: 更新偏移并重发
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
                        for (int i = 3; i >= 0; --i)
                            m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
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
                if (type == ZRINIT || type == ZSKIP) {
                    m_timeoutTimer->stop();
                    m_retryCount = 0;
                    m_zmodemState = State::SendingFin;
                    sendZFIN();
                    m_timeoutTimer->start(m_timeoutMs);
                }
                break;

            case State::SendingFin:
                if (type == ZFIN) {
                    m_timeoutTimer->stop();
                    if (m_conn) m_conn->write(QByteArray("OO"));
                    emit progress(100, m_fileData.size(), m_fileData.size());
                    finishTransfer();
                }
                break;
            }
        } else {
            // 数据不足或缓冲区过大时清空
            if (m_receiveBuffer.size() > 4096) {
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

// ---- 帧解析(含CRC16校验) ----

/** @brief 解析HEX帧并验证CRC16, 格式: ZPAD ZDLE ZHEX <type:2hex> <f0-f3:8hex> <crc16:4hex> */
bool ZModemTransfer::parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData)
{
    if (data.size() < 7) return false;

    int idx = 0;
    if (data[idx] != ZPAD) return false;
    idx++;
    if (idx >= data.size() || static_cast<quint8>(data[idx]) != ZDLE) return false;
    idx++;
    if (idx >= data.size() || data[idx] != ZHEX) return false;
    idx++;
    if (idx + 14 > data.size()) return false;

    // HEX字符转数值辅助
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    auto readHexByte = [&](int& offset) -> int {
        int hi = hexVal(data[offset]), lo = hexVal(data[offset + 1]);
        if (hi < 0 || lo < 0) return -1;
        offset += 2;
        return (hi << 4) | lo;
    };

    // 读取帧类型(1字节)
    int typeVal = readHexByte(idx);
    if (typeVal < 0) return false;
    type = typeVal;

    // 读取帧头数据(4字节)并构建CRC计算输入
    headerData.clear();
    QByteArray crcInput;
    crcInput.append(static_cast<char>(typeVal));
    for (int i = 0; i < 4; ++i) {
        int b = readHexByte(idx);
        if (b < 0) return false;
        headerData.append(static_cast<char>(b));
        crcInput.append(static_cast<char>(b));
    }

    // 读取CRC16(2字节)并校验
    int crcHi = readHexByte(idx);
    int crcLo = readHexByte(idx);
    if (crcHi < 0 || crcLo < 0) return false;

    quint16 receivedCrc = static_cast<quint16>((crcHi << 8) | crcLo);
    quint16 calculatedCrc = CRC::crc16Ccitt(crcInput);

    if (receivedCrc != calculatedCrc) {
        qWarning() << "ZModem: CRC16 mismatch, rx:" << Qt::hex << receivedCrc
                   << "calc:" << calculatedCrc;
        m_receiveBuffer.remove(0, idx);
        return false;
    }

    // 跳过CR/LF和可能的ZDLE行结束
    while (idx < data.size() && (data[idx] == '\r' || data[idx] == '\n'))
        idx++;
    if (idx < data.size() && static_cast<quint8>(data[idx]) == ZDLE) {
        idx++;
        if (idx < data.size()) idx++;
    }

    m_receiveBuffer.remove(0, idx);
    return true;
}

// ---- 帧构建 ----

/** @brief 构建HEX帧头: ZPAD ZDLE ZHEX + hex(type+data+crc16) + CRLF */
QByteArray ZModemTransfer::buildHexHeader(quint8 frameType, const QByteArray& data)
{
    QByteArray frame;
    frame.append(ZPAD);
    frame.append(ZDLE);
    frame.append(ZHEX);

    QByteArray payload;
    payload.append(static_cast<char>(frameType));
    for (int i = 0; i < 4; ++i)
        payload.append(i < data.size() ? data[i] : '\0');

    quint16 crc = CRC::crc16Ccitt(payload);
    payload.append(static_cast<char>((crc >> 8) & 0xFF));
    payload.append(static_cast<char>(crc & 0xFF));

    for (char b : payload)
        frame.append(toHex(static_cast<quint8>(b), 2));
    frame.append("\r\n");
    return frame;
}

/** @brief 构建BIN32帧头: ZPAD ZDLE ZBIN32 + ZDLE转义(type+data+crc32) */
QByteArray ZModemTransfer::buildBinHeader(quint8 frameType, const QByteArray& data)
{
    QByteArray frame;
    frame.append(ZPAD);
    frame.append(ZDLE);
    frame.append(ZBIN32);

    QByteArray payload;
    payload.append(static_cast<char>(frameType));
    for (int i = 0; i < 4; ++i)
        payload.append(i < data.size() ? data[i] : '\0');

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

/** @brief 构建数据子帧: 转义数据 + ZDLE endFlag + 转义CRC32 */
QByteArray ZModemTransfer::buildDataSubpacket(char endFlag, const QByteArray& data)
{
    QByteArray packet;
    // 数据ZDLE转义
    for (char b : data) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 || c == 0x2A) {
            packet.append(ZDLE);
            packet.append(static_cast<char>(c ^ 0x40));
        } else {
            packet.append(b);
        }
    }
    // CRC32 = crc32(data + endFlag)
    QByteArray crcInput = data;
    crcInput.append(endFlag);
    quint32 crc = CRC::crc32(crcInput);

    packet.append(ZDLE);
    packet.append(endFlag);

    // CRC32字节ZDLE转义
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

// ---- 发送流程方法 ----

void ZModemTransfer::sendZRQINIT() { if (m_conn) m_conn->write(buildHexHeader(ZRQINIT)); }

/** @brief 发送ZFILE: BIN32帧头 + 文件名/大小数据子帧 */
void ZModemTransfer::sendZFILE()
{
    if (!m_conn) return;
    m_conn->write(buildBinHeader(ZFILE));
    QFileInfo info(m_filePath);
    QByteArray fi = QString("%1 %2 0").arg(info.fileName()).arg(info.size()).toUtf8();
    fi.append('\0');
    m_conn->write(buildDataSubpacket(ZCRCW, fi));
}

/** @brief 发送ZDATA: BIN32帧头携带当前偏移(小端序) */
void ZModemTransfer::sendZDATA()
{
    if (!m_conn) return;
    QByteArray offsetData;
    offsetData.append(static_cast<char>(m_fileOffset & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 8) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 16) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 24) & 0xFF));
    m_conn->write(buildBinHeader(ZDATA, offsetData));
}

/** @brief 批量发送数据子帧, 中间ZCRCG(连续), 最后ZCRCW(等待ACK) */
void ZModemTransfer::sendDataSubpackets()
{
    if (!m_conn) return;
    sendZDATA();

    qint64 offset = m_fileOffset;
    int lastPercent = static_cast<int>((offset * 100) / qMax(m_fileData.size(), qint64(1)));
    while (offset < m_fileData.size()) {
        int chunkSize = qMin(static_cast<int>(m_fileData.size() - offset), kDataLen);
        QByteArray chunk = m_fileData.mid(offset, chunkSize);
        bool isLast = (offset + chunkSize >= m_fileData.size());
        char endFlag = isLast ? ZCRCW : ZCRCG;
        m_conn->write(buildDataSubpacket(endFlag, chunk));

        offset += chunkSize;
        m_bytesSent = offset;
        int percent = static_cast<int>((offset * 100) / m_fileData.size());
        // 每 1% 变化或最后一帧才发射进度信号，避免 UI 线程被高频信号淹没
        if (percent != lastPercent || isLast) {
            emit progress(percent, offset, m_fileData.size());
            lastPercent = percent;
        }
    }

    m_fileOffset = offset;
    m_bytesSent = offset;
    if (m_bytesSent >= m_fileData.size()) {
        m_zmodemState = State::WaitingZAck;
        m_timeoutTimer->start(m_timeoutMs);
    }
}

/** @brief 发送ZEOF: HEX帧携带文件大小(小端序) */
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

void ZModemTransfer::sendZFIN() { if (m_conn) m_conn->write(buildHexHeader(ZFIN)); }

// ---- 工具方法 ----

quint32 ZModemTransfer::encodeCrc32(quint32 crc) { return crc; }
/** @brief 数值转大写HEX ASCII字符串 */
QByteArray ZModemTransfer::toHex(quint32 val, int digits)
{
    QByteArray result;
    for (int i = digits - 1; i >= 0; --i) {
        int nibble = (val >> (i * 4)) & 0xF;
        result.append(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10));
    }
    return result;
}

void ZModemTransfer::setState(State s) { m_zmodemState = s; }
