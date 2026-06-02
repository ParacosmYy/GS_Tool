/**
 * @file ZModemTransfer.cpp
 * @brief ZMODEM协议传输器实现
 *
 * 实现ZMODEM Sender端状态机: HEX帧解析(16位CRC校验)、BIN32帧构建、
 * 数据子帧发送(ZCRCG/ZCRCW)、超时重发、断点续传(ZRPOS偏移处理)
 */
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include <QFile>
#include <QFileInfo>
#include <QTimer>
/** @brief 将状态枚举转换为可读字符串
 *  @param s ZMODEM状态枚举值
 *  @return 状态名称字符串 */
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
    default:                  return QStringLiteral("Unknown");
    }
}
/** @brief 构造函数，初始化ZMODEM传输器并设置默认超时10秒 */
ZModemTransfer::ZModemTransfer(QObject* parent)
    : BaseTransfer(parent)
{
    m_timeoutMs = 10000;
}
/** @brief 设置传输文件路径
 *  @param path 文件绝对路径 */
void ZModemTransfer::setFilePath(const QString& path) { m_filePath = path; }

/** @brief 安全写入: 检测write返回值，连接断开时立即终止传输
 *  @param data 待写入的数据
 *  @return true=成功写入, false=连接已断开(已设置Error状态) */
bool ZModemTransfer::writeChecked(const QByteArray& data)
{
    if (!m_conn) {
        m_zmodemState = State::Error;
        markError();
        emit transferError(tr("连接中断: 连接对象无效"));
        return false;
    }
    qint64 written = m_conn->write(data);
    if (written < 0) {
        m_zmodemState = State::Error;
        markError();
        emit transferError(
            tr("连接中断: 写入失败, 已传输 %1/%2 字节")
                .arg(m_bytesSent)
                .arg(m_fileData.size()));
        return false;
    }
    return true;
}

// ---- BaseTransfer钩子实现 ----
/** @brief 传输启动初始化，校验文件并读取到内存，发送ZRQINIT开始握手
 *  @return 初始化成功返回true，文件不存在或读取失败返回false */
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
    // 文件大小校验(最大16MB)
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
        emit transferError(tr("读取文件失败"));
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

/** @brief 发送取消序列: 8个退格符+2个CAN字节，中断ZMODEM传输 */
void ZModemTransfer::sendCancelBytes()
{
    if (m_conn) {
        m_conn->write(QByteArray(8, 0x08));
        m_conn->write(QByteArray(2, static_cast<char>(0x18)));
    }
}

/** @brief 超时处理，根据当前状态重发对应帧(ZRQINIT/ZFILE/ZDATA/ZEOF/ZFIN) */
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
        sendDataSubpackets();  // 重发数据子包（内部已启动定时器）
        break;
    case State::SendingEof:
        qWarning() << "ZModem: timeout in" << curState << "- retrying ZEOF";
        sendZEOF();
        m_timeoutTimer->start(m_timeoutMs);  // 重启定时器等待ZRINIT响应
        break;
    default:
        qWarning() << "ZModem: unexpected timeout in state" << curState;
        break;
    }
}

/** @brief 处理接收缓冲区数据，解析HEX帧并分发给对应状态处理方法 */
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
            default:
                qWarning() << "ZModem: unknown state" << static_cast<int>(m_zmodemState);
                return;
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

// ---- 帧解析(含CRC16校验) ----
/** @brief 解析ZMODEM HEX帧，提取帧类型和帧头数据并校验CRC16
 *  @param data 接收缓冲区数据
 *  @param type 输出帧类型
 *  @param headerData 输出帧头4字节数据
 *  @return 解析成功返回true，数据不完整或CRC校验失败返回false */
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
// ---- 发送流程方法 ----
/** @brief 发送ZRQINIT帧，发起ZMODEM传输握手 */
void ZModemTransfer::sendZRQINIT() { if (m_conn) writeChecked(buildHexHeader(ZRQINIT)); }
/** @brief 发送ZFILE帧(文件名+大小)和数据子包，通知接收方文件信息 */
void ZModemTransfer::sendZFILE()
{
    if (!m_conn) return;
    if (!writeChecked(buildBinHeader(ZFILE))) return;
    QFileInfo info(m_filePath);
    QByteArray fi = QString("%1 %2 0").arg(info.fileName()).arg(info.size()).toUtf8();
    fi.append('\0');
    writeChecked(buildDataSubpacket(ZCRCW, fi));
}
/** @brief 发送ZDATA帧头，包含当前文件偏移量 */
void ZModemTransfer::sendZDATA()
{
    if (!m_conn) return;
    QByteArray offsetData;
    offsetData.append(static_cast<char>(m_fileOffset & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 8) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 16) & 0xFF));
    offsetData.append(static_cast<char>((m_fileOffset >> 24) & 0xFF));
    writeChecked(buildBinHeader(ZDATA, offsetData));
}
/** @brief 异步分块发送数据子包，每批最多kChunksPerTick个，防止UI冻结 */
void ZModemTransfer::sendDataSubpackets()
{
    // 取消或空闲时立即停止发送，防止CAN字节后继续发送数据
    if (!m_conn || m_cancelled || isIdle()) return;
    sendZDATA();
    // 异步分块发送: 每次最多kChunksPerTick个子包，然后让出事件循环
    // 防止大数据传输时UI冻结和取消按钮无响应
    static const int kChunksPerTick = 32;
    qint64 offset = m_fileOffset;
    int lastPct = static_cast<int>((offset * 100) / qMax(m_fileData.size(), qint64(1)));
    int sent = 0;
    while (offset < m_fileData.size() && sent < kChunksPerTick) {
        int chunkSize = qMin(static_cast<int>(m_fileData.size() - offset), kDataLen);
        QByteArray chunk = m_fileData.mid(offset, chunkSize);
        bool isLast = (offset + chunkSize >= m_fileData.size());
        char endFlag = isLast ? ZCRCW : ZCRCG;
        if (!writeChecked(buildDataSubpacket(endFlag, chunk))) return;
        offset += chunkSize;
        m_bytesSent = offset;
        int pct = static_cast<int>((offset * 100) / m_fileData.size());
        if (pct != lastPct || isLast) {
            emit progress(pct, offset, m_fileData.size());
            lastPct = pct;
        }
        sent++;
    }
    m_fileOffset = offset;
    m_bytesSent = offset;
    if (m_bytesSent >= m_fileData.size()) {
        // 所有数据发送完成，等待接收方确认
        m_zmodemState = State::WaitingZAck;
        m_timeoutTimer->start(m_timeoutMs);
    } else {
        // 还有数据未发送，让出事件循环后继续发送下一批
        QTimer::singleShot(0, this, &ZModemTransfer::sendDataSubpackets);
    }
}
/** @brief 发送ZEOF帧，通知接收方文件传输完成 */
void ZModemTransfer::sendZEOF()
{
    if (!m_conn) return;
    QByteArray offsetData;
    qint64 size = m_fileData.size();
    offsetData.append(static_cast<char>(size & 0xFF));
    offsetData.append(static_cast<char>((size >> 8) & 0xFF));
    offsetData.append(static_cast<char>((size >> 16) & 0xFF));
    offsetData.append(static_cast<char>((size >> 24) & 0xFF));
    writeChecked(buildHexHeader(ZEOF, offsetData));
}
/** @brief 发送ZFIN帧，结束ZMODEM会话 */
void ZModemTransfer::sendZFIN() { if (m_conn) writeChecked(buildHexHeader(ZFIN)); }
/** @brief 设置ZMODEM状态机状态
 *  @param s 目标状态 */
void ZModemTransfer::setState(State s) { m_zmodemState = s; }
