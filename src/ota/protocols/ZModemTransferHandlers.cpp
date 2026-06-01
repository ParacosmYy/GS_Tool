/**
 * @file ZModemTransferHandlers.cpp
 * @brief ZMODEM协议状态处理器与帧构建方法 - 协议状态机各状态处理 + 帧构建工具
 *
 * 从ZModemTransfer.cpp拆分而来，包含:
 *   - 6个状态处理方法: 由processReceivedData()根据协议状态分发调用
 *   - 3个帧构建方法: HEX帧头(16进制ASCII+CRC16)、BIN32帧头(二进制+CRC32)、数据子包
 *   - 2个工具方法: toHex数值转换、escapeZdle ZDLE转义编码
 */
#include "ota/protocols/ZModemTransfer.h"
#include "utils/CRC.h"

// ---- 状态处理方法 ----
/** @brief 处理WaitingRinit状态: 收到ZRINIT后发送ZFILE开始文件传输
 *  @param type 解析到的帧类型 */
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
/** @brief 处理SendingFile状态: 解析ZRPOS断点续传/ZSKIP跳过/ZRINIT重发
 *  @param type 解析到的帧类型
 *  @param headerData 帧头数据(含ZRPOS偏移量) */
void ZModemTransfer::handleStateSendingFile(int type, const QByteArray& headerData){
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
/** @brief 处理SendingData状态: 解析ZRPOS重传请求/ZACK确认，超过重试上限则取消
 *  @param type 解析到的帧类型
 *  @param headerData 帧头数据(含ZRPOS偏移量) */
void ZModemTransfer::handleStateSendingData(int type, const QByteArray& headerData)
{
    if (type == ZRPOS) {
        m_timeoutTimer->stop();
        m_retryCount++;
        if (m_retryCount > m_maxRetries) {
            sendCancelBytes();
            m_zmodemState = State::Error;
            markError();  // 通知BaseTransfer状态已转为Error，否则isRunning()永远为true
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
/** @brief 处理WaitingZAck状态: 收到ZACK/ZRPOS后发送ZEOF结束文件数据传输
 *  @param type 解析到的帧类型 */
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
/** @brief 处理SendingEof状态: 收到ZRINIT/ZSKIP后发送ZFIN结束会话
 *  @param type 解析到的帧类型 */
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
/** @brief 处理SendingFin状态: 收到ZFIN后发送"OO"结束序列并完成传输
 *  @param type 解析到的帧类型 */
void ZModemTransfer::handleStateSendingFin(int type)
{
    if (type == ZFIN) {
        m_timeoutTimer->stop();
        if (m_conn) writeChecked(QByteArray("OO"));
        emit progress(100, m_fileData.size(), m_fileData.size());
        finishTransfer();
    } else {
        qWarning() << "ZModem: unexpected frame" << type << "in SendingFin";
    }
}

// ---- 帧构建方法 ----

/** @brief 构建ZMODEM HEX格式帧头(16进制ASCII编码+CRC16校验)
 *  @param frameType 帧类型(ZRQINIT/ZRINIT/ZEOF/ZFIN等)
 *  @param data 帧头附加数据(4字节，不足补零)
 *  @return 完整的HEX帧字节数组 */
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

/** @brief 构建ZMODEM BIN32格式帧头(二进制编码+CRC32校验+ZDLE转义)
 *  @param frameType 帧类型(ZFILE/ZDATA等)
 *  @param data 帧头附加数据(4字节)
 *  @return 完整的BIN32帧字节数组 */
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
    // ZDLE转义: 控制字符 + 前8字节载荷(type+4B data)内的0x00
    // 使用payload字节索引而非frame.size()，避免ZDLE扩展导致索引偏移
    int payloadIdx = 0;
    for (char b : payload) {
        quint8 c = static_cast<quint8>(b);
        if (c == 0x00 && payloadIdx < 8) {  // 前8字节(type+4B data)中的NUL需要转义
            frame.append(ZDLE);
            frame.append(static_cast<char>(c ^ 0x40));
        } else {
            frame.append(escapeZdle(QByteArray(1, b)));
        }
        ++payloadIdx;
    }
    return frame;
}

/** @brief 构建数据子包(ZDLE转义数据+CRC32校验+结束标志)
 *  @param endFlag 结束标志: ZCRCG(继续)/ZCRCW(等待应答)
 *  @param data 子包数据载荷
 *  @return 完整的数据子包字节数组 */
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

// ---- 工具方法 ----

/** @brief 将数值转换为指定位数的16进制大写ASCII字符串
 *  @param val 待转换的数值
 *  @param digits 16进制位数
 *  @return 16进制ASCII字节数组 */
QByteArray ZModemTransfer::toHex(quint32 val, int digits)
{
    QByteArray result;
    for (int i = digits - 1; i >= 0; --i) {
        int nibble = (val >> (i * 4)) & 0xF;
        result.append(nibble < 10 ? ('0' + nibble) : ('A' + nibble - 10));
    }
    return result;
}

/** @brief 对数据进行ZDLE转义编码，转义控制字符(CAN/CR/LF/XON/XOFF/0x2A)
 *  @param data 原始数据
 *  @return 转义后的数据 */
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
