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

QString ZModemTransfer::stateToString(State s)
{
    switch (s) {
    case State::Idle:         return QStringLiteral("Idle");
    case State::WaitingRinit: return QStringLiteral("WaitingRinit");
    case State::SendingFile:  return QStringLiteral("SendingFile");
    case State::SendingData:  return QStringLiteral("SendingData");
    case State::WaitingZAck:  return QStringLiteral("WaitingZAck");
    case State::SendingEof:   return QStringLiteral("SendingEof");
    case State::SendingFin:   return QStringLiteral("SendingFin");
    case State::Done:         return QStringLiteral("Done");
    case State::Error:        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;
}

void ZModemTransfer::setFilePath(const QString& path) { m_filePath = path; }

// ---- BaseTransfer钩子实现 ----
bool ZModemTransfer::onStartInit()
{
    // 文件路径非空校验
    if (m_filePath.isEmpty()) {
        emit transferError(tr("未设置文件路径，请先调用 setFilePath()"));
        return false;
    }
    // 文件存在性校验
    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists()) {
        emit transferError(tr("文件不存在: %1").arg(m_filePath));
        return false;
    }
    // 文件大小校验(最大1MB)
    if (fileInfo.size() > BaseTransfer::kMaxFileSize) {
        emit transferError(tr("文件过大: %1 (%2 字节, 上限 %3 字节)")
                               .arg(m_filePath).arg(fileInfo.size()).arg(BaseTransfer::kMaxFileSize));
        return false;
    }
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferError(tr("无法打开文件: %1").arg(m_filePath));
        return false;
    }
    m_fileData = file.readAll();
    if (m_fileData.size() != fileInfo.size()) {
        emit transferError(tr("Failed to read file"));
        return false;
    }
    file.close();
    // 空文件无法传输
    if (m_fileData.isEmpty()) {
        emit transferError(tr("文件为空，无法传输: %1").arg(m_filePath));
        return false;
    }
    m_bytesSent = 0;
    m_fileOffset = 0;
    m_senderCrc32 = 0;
    m_zmodemState = State::WaitingRinit;
    sendZRQINIT();
    m_timeoutTimer->start(m_timeoutMs);
    return true;
}

void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));
        m_conn->write(QByteArray(2, static_cast<char>(0x18)));
    }
}

void ZModemTransfer::handleTimeout()
{
    QString curState = stateToString(m_zmodemState);
    switch (m_zmodemState) {
    case State::WaitingRinit:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZRQINIT, attempt" << m_retryCount;
        sendZRQINIT();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFile:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZFILE, attempt" << m_retryCount;
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::WaitingZAck:
        qWarning() << "ZModem: timeout in" << curState << "- bytes sent:" << m_bytesSent;
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingFin:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZFIN, attempt" << m_retryCount;
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
        break;
    case State::SendingData:
        qWarning() << "ZModem: timeout in" << curState << "- offset:" << m_fileOffset << "bytes:" << m_bytesSent;
        break;
    case State::SendingEof:
        qWarning() << "ZModem: timeout in" << curState << "- file size:" << m_fileData.size();
        break;
    default:
        qWarning() << "ZModem: unexpected timeout in state" << curState;
        break;
    }
}

void ZModemTransfer::processReceivedData()
{
    while (!m_receiveBuffer.isEmpty()) {
        if (m_cancelled) return;
        int padIdx = m_receiveBuffer.indexOf(ZPAD);
        if (padIdx < 0) { m_receiveBuffer.clear(); return; }
        if (padIdx > 0) m_receiveBuffer.remove(0, padIdx);

        int type = -1;
        QByteArray headerData;
        if (parseHexFrame(m_receiveBuffer, type, headerData)) {
            switch (m_zmodemState) {
            case State::Idle:
            case State::Done:
            case State::Error:
                return;
            case State::WaitingRinit:
                handleStateWaitingRinit(type);
                break;
            case State::SendingFile:
                handleStateSendingFile(type, headerData);
                break;
            case State::SendingData:
                handleStateSendingData(type, headerData);
                if (m_zmodemState == State::Error) return;
                break;
            case State::WaitingZAck:
                handleStateWaitingZAck(type);
                break;
            case State::SendingEof:
                handleStateSendingEof(type);
                break;
            case State::SendingFin:
                handleStateSendingFin(type);
                break;
            }
        } else {
            // 缓冲区过大(>4096)且无法解析时清空，防止垃圾数据堆积
            if (m_receiveBuffer.size() > 4096) {
                qWarning() << "ZModem: buffer overflow (>4096) in state"
                           << stateToString(m_zmodemState) << "- clearing";
                m_receiveBuffer.clear();
            }
            return;
        }
    }
}

// ---- 状态处理方法 ----
void ZModemTransfer::handleStateWaitingRinit(int type)
{
    if (type == ZRINIT) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingFile;
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in WaitingRinit";
    }
}
void ZModemTransfer::handleStateSendingFile(int type, const QByteArray& headerData)
{
    if (type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        if (headerData.size() >= 4) {
            m_fileOffset = 0;
            for (int i = 3; i >= 0; --i)
                m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
        }
        qWarning() << "ZModem: ZRPOS resume at offset" << m_fileOffset;
        m_bytesSent = m_fileOffset;
        m_zmodemState = State::SendingData;
        sendDataSubpackets();
    } else if (type == ZSKIP) {
        m_timeoutTimer->stop();
        qWarning() << "ZModem: receiver skipped file";
        m_zmodemState = State::SendingFin;
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
    } else if (type == ZRINIT) {
        m_timeoutTimer->stop();
        sendZFILE();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingFile";
    }
}
void ZModemTransfer::handleStateSendingData(int type, const QByteArray& headerData)
{
    if (type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount++;
        if (m_retryCount > m_maxRetries) {
            sendCancelBytes();
            m_zmodemState = State::Error;
            emit transferError(
                tr("重传请求次数过多 (已重试 %1 次, 上限 %2 次), "
                   "当前偏移: %3 字节, 文件大小: %4 字节")
                    .arg(m_retryCount).arg(m_maxRetries)
                    .arg(m_fileOffset).arg(m_fileData.size()));
            return;
        }
        if (headerData.size() >= 4) {
            m_fileOffset = 0;
            for (int i = 3; i >= 0; --i)
                m_fileOffset = (m_fileOffset << 8) | static_cast<quint8>(headerData[i]);
        }
        qWarning() << "ZModem: ZRPOS retransmit at offset" << m_fileOffset
                   << "retry" << m_retryCount << "/" << m_maxRetries;
        m_bytesSent = m_fileOffset;
        sendDataSubpackets();
    } else if (type == ZACK) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingData, offset:" << m_fileOffset;
    }
}
void ZModemTransfer::handleStateWaitingZAck(int type)
{
    if (type == ZACK || type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingEof;
        sendZEOF();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in WaitingZAck, bytes:" << m_bytesSent;
    }
}
void ZModemTransfer::handleStateSendingEof(int type)
{
    if (type == ZRINIT || type == ZSKIP) {
        m_timeoutTimer->stop();
        m_retryCount = 0;
        m_zmodemState = State::SendingFin;
        sendZFIN();
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingEof";
    }
}
void ZModemTransfer::handleStateSendingFin(int type)
{
    if (type == ZFIN) {
        m_timeoutTimer->stop();
        if (m_conn) m_conn->write(QByteArray("OO"));
        emit progress(100, m_fileData.size(), m_fileData.size());
        finishTransfer();
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingFin";
    }
}

// ---- 帧解析(含CRC16校验) ----
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
    // HEX字符转数值辅助lambda
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
    int typeVal = readHexByte(idx);
    if (typeVal < 0) return false;
    type = typeVal;
    headerData.clear();
    QByteArray crcInput;
    crcInput.append(static_cast<char>(typeVal));
    for (int i = 0; i < 4; ++i) {
        int b = readHexByte(idx);
        if (b < 0) return false;
        headerData.append(static_cast<char>(b));
        crcInput.append(static_cast<char>(b));
    }
    int crcHi = readHexByte(idx);
    int crcLo = readHexByte(idx);
    if (crcHi < 0 || crcLo < 0) return false;
    quint16 receivedCrc = static_cast<quint16>((crcHi << 8) | crcLo);
    quint16 calculatedCrc = CRC::crc16Ccitt(crcInput);
    if (receivedCrc != calculatedCrc) {
        qWarning() << "ZModem: CRC16 mismatch frame" << type
                   << "rx:" << Qt::hex << receivedCrc << "calc:" << calculatedCrc
                   << "state:" << stateToString(m_zmodemState);
        m_receiveBuffer.remove(0, idx);
        return false;
    }
    // 跳过CR/LF和可能的ZDLE行结束标记
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
    // ZDLE转义: 控制字符 + 帧头前8字节内的0x00
    for (char b : payload) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x00 && frame.size() < 8) {
            frame.append(ZDLE);
            frame.append(static_cast<char>(c ^ 0x40));
        } else {
            frame.append(escapeZdle(QByteArray(1, b)));
        }
    }
    return frame;
}

QByteArray ZModemTransfer::buildDataSubpacket(char endFlag, const QByteArray& data)
{
    QByteArray packet;
    packet.append(escapeZdle(data));
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
    packet.append(escapeZdle(crcBytes));
    return packet;
}

// ---- 发送流程方法 ----
void ZModemTransfer::sendZRQINIT() { if (m_conn) m_conn->write(buildHexHeader(ZRQINIT)); }

void ZModemTransfer::sendZFILE()
{
    if (!m_conn) return;
    m_conn->write(buildBinHeader(ZFILE));
    QFileInfo info(m_filePath);
    QByteArray fi = QString("%1 %2 0").arg(info.fileName()).arg(info.size()).toUtf8();
    fi.append('\0');
    m_conn->write(buildDataSubpacket(ZCRCW, fi));
}

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
void ZModemTransfer::setState(State s) { m_zmodemState = s; }

QByteArray ZModemTransfer::toHex(quint32 val, int digits)
{
    QByteArray result;
    for (int i = digits - 1; i >= 0; --i) {
        int nibble = (val >> (i * 4)) & 0xF;
        result.append(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10));
    }
    return result;
}

QByteArray ZModemTransfer::escapeZdle(const QByteArray& data) const
{
    QByteArray result;
    for (char b : data) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x18 || c == 0x0D || c == 0x0A || c == 0x11 || c == 0x13 || c == 0x2A) {
            result.append(ZDLE);
            result.append(static_cast<char>(c ^ 0x40));
        } else {
            result.append(b);
        }
    }
    return result;
}
