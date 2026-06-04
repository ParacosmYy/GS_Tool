/**
 * @file ZModemTransferFrames.cpp
 * @brief ZMODEM协议帧构建与工具方法 - HEX/BIN32帧头构建 + 数据子包 + ZDLE转义
 *
 * 从ZModemTransferHandlers.cpp拆分而来，包含:
 *   - 3个帧构建方法: HEX帧头(16进制ASCII+CRC16)、BIN32帧头(二进制+CRC32)、数据子包
 *   - 2个工具方法: toHex数值转换、escapeZdle ZDLE转义编码
 *
 * 状态处理方法保留在ZModemTransferHandlers.cpp中。
 */

#include "ota/protocols/zmodem/ZModemTransfer.h"
#include "utils/crypto/CRC.h"

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
    // ZDLE转义: 所有控制字符(0x00-0x1F)和0x2A统一通过escapeZdle处理
    for (char b : payload) {
        frame.append(escapeZdle(QByteArray(1, b)));
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

/** @brief 对数据进行ZDLE转义编码，转义控制字符(CAN/CR/LF/XON/XOFF/0x2A)及0x00-0x1F
 *  @param data 原始数据
 *  @return 转义后的数据 */
QByteArray ZModemTransfer::escapeZdle(const QByteArray& data) const
{
    QByteArray result;
    for (char b : data) {
        quint8 c = static_cast<quint8>(b);
        if (c <= 0x1F || c == 0x2A) {
            result.append(ZDLE);
            result.append(static_cast<char>(c ^ 0x40));
        } else {
            result.append(b);
        }
    }
    return result;
}
