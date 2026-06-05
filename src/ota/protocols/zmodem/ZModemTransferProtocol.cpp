/**
 * @file ZModemTransferProtocol.cpp
 * @brief ZMODEM协议帧解析与发送流程方法
 *
 * 从ZModemTransfer.cpp拆分而来，包含:
 *   - 帧解析: HEX帧解析(16位CRC校验)、帧类型提取
 *   - 发送流程: ZRQINIT/ZFILE/ZDATA/ZEOF/ZFIN帧发送、数据子包批量异步发送
 */
#include "ota/protocols/zmodem/ZModemTransfer.h"
#include <QFileInfo>
#include <QTimer>
#include "utils/crypto/CRC.h"

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
        ++m_stats.crcErrors;  ///< 统计: CRC校验失败
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
    ++m_stats.zfileSent;
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
    ++m_stats.zdataFrames;
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
        int pct = static_cast<int>((offset * 100) / qMax(m_fileData.size(), qint64(1)));
        if (pct != lastPct || isLast) {
            emit progress(pct, offset, m_fileData.size());
            lastPct = pct;
        }
        sent++;
        ++m_stats.blocksSent;  ///< 统计: 每发送一个数据块
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
void ZModemTransfer::sendZFIN() { if (m_conn) { ++m_stats.zfinSent; writeChecked(buildHexHeader(ZFIN)); } }
