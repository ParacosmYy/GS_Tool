/**
 * @file SerialFrameDecoderBuild.cpp
 * @brief 通用串口帧解码器 — 帧组装/字段注入/校验和计算
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/decoder/SerialFrameDecoder.h"

// ============================================================
// 帧组装 (Build)
// ============================================================

/**
 * @brief 根据字段值和帧定义组装完整帧
 *
 * 先分配最小长度的缓冲区，填入帧头 -> 字段值 -> 校验和 -> 帧尾。
 * 成功时发射 frameBuilt 信号。
 */
QByteArray SerialFrameDecoder::buildFrame(const QVariantMap &fieldValues)
{
    // 计算帧总长度
    int totalLen = minimumFrameLength();

    // 确定是否有可变长 Bytes 字段需要额外空间
    for (const FieldDef &field : m_frameDef.fields) {
        if (field.type == FieldType::Bytes) {
            const QVariant val = fieldValues.value(field.name);
            const int actualLen = val.toByteArray().size();
            const int fixedLen = fixedFieldSize(field.type); // Bytes => 0
            if (actualLen > fixedLen) {
                totalLen += (actualLen - field.length);
            }
        }
    }

    QByteArray frame(totalLen, 0x00);
    int pos = 0;

    // 写入帧头
    if (!m_frameDef.header.isEmpty()) {
        std::memcpy(frame.data() + pos, m_frameDef.header.constData(),
                    m_frameDef.header.size());
        pos += m_frameDef.header.size();
    }

    // 写入各字段值
    for (const FieldDef &field : m_frameDef.fields) {
        const QVariant val = fieldValues.value(field.name);
        injectFieldValue(frame, field, val);
    }

    // 写入帧尾
    if (!m_frameDef.footer.isEmpty()) {
        std::memcpy(frame.data() + totalLen - m_frameDef.footer.size(),
                    m_frameDef.footer.constData(),
                    m_frameDef.footer.size());
    }

    // 写入校验和
    if (m_frameDef.checksumType != ChecksumType::None && m_frameDef.checksumOffset >= 0) {
        const int csLen = qMax(1, m_frameDef.checksumLength);
        const QPair<int, int> range = checksumRange(totalLen);
        const QByteArray payloadSlice = frame.mid(range.first, range.second - range.first);
        const QByteArray cs = computeChecksum(payloadSlice, m_frameDef.checksumType, csLen);
        std::memcpy(frame.data() + m_frameDef.checksumOffset, cs.constData(), csLen);
    }

    // 更新统计
    m_stats.totalFramesBuilt++;
    m_totalFrameSizeForAvg += frame.size();
    const quint64 totalFrames = m_stats.totalFramesDecoded + m_stats.totalFramesBuilt;
    m_stats.avgFrameSize = (totalFrames > 0)
                               ? static_cast<double>(m_totalFrameSizeForAvg) / static_cast<double>(totalFrames)
                               : 0.0;

    emit frameBuilt(frame);
    return frame;
}

// ============================================================
// 内部组装辅助 — 字段注入
// ============================================================

/**
 * @brief 将字段值写入字节数组的指定位置
 *
 * 根据字段类型和字节序将 QVariant 转换为原始字节并写入缓冲区。
 */
void SerialFrameDecoder::injectFieldValue(QByteArray &buf,
                                           const FieldDef &field,
                                           const QVariant &value) const
{
    const bool bigEndian = (m_frameDef.endianness == Endianness::BigEndian);
    char *dst = buf.data() + field.offset;

    switch (field.type) {
    case FieldType::UInt8: {
        const quint8 v = static_cast<quint8>(value.toUInt());
        std::memcpy(dst, &v, 1);
        break;
    }
    case FieldType::UInt16: {
        quint16 v = value.toUInt();
        if (bigEndian) {
            dst[0] = static_cast<char>((v >> 8) & 0xFF);
            dst[1] = static_cast<char>(v & 0xFF);
        } else {
            dst[0] = static_cast<char>(v & 0xFF);
            dst[1] = static_cast<char>((v >> 8) & 0xFF);
        }
        break;
    }
    case FieldType::UInt32: {
        quint32 v = value.toUInt();
        if (bigEndian) {
            dst[0] = static_cast<char>((v >> 24) & 0xFF);
            dst[1] = static_cast<char>((v >> 16) & 0xFF);
            dst[2] = static_cast<char>((v >> 8) & 0xFF);
            dst[3] = static_cast<char>(v & 0xFF);
        } else {
            dst[0] = static_cast<char>(v & 0xFF);
            dst[1] = static_cast<char>((v >> 8) & 0xFF);
            dst[2] = static_cast<char>((v >> 16) & 0xFF);
            dst[3] = static_cast<char>((v >> 24) & 0xFF);
        }
        break;
    }
    case FieldType::Int8: {
        const qint8 v = static_cast<qint8>(value.toInt());
        std::memcpy(dst, &v, 1);
        break;
    }
    case FieldType::Int16: {
        qint16 v = static_cast<qint16>(value.toInt());
        if (bigEndian) {
            dst[0] = static_cast<char>((v >> 8) & 0xFF);
            dst[1] = static_cast<char>(v & 0xFF);
        } else {
            dst[0] = static_cast<char>(v & 0xFF);
            dst[1] = static_cast<char>((v >> 8) & 0xFF);
        }
        break;
    }
    case FieldType::Int32: {
        qint32 v = value.toInt();
        if (bigEndian) {
            dst[0] = static_cast<char>((v >> 24) & 0xFF);
            dst[1] = static_cast<char>((v >> 16) & 0xFF);
            dst[2] = static_cast<char>((v >> 8) & 0xFF);
            dst[3] = static_cast<char>(v & 0xFF);
        } else {
            dst[0] = static_cast<char>(v & 0xFF);
            dst[1] = static_cast<char>((v >> 8) & 0xFF);
            dst[2] = static_cast<char>((v >> 16) & 0xFF);
            dst[3] = static_cast<char>((v >> 24) & 0xFF);
        }
        break;
    }
    case FieldType::Float32: {
        float f = static_cast<float>(value.toDouble());
        if (bigEndian) {
            char tmp[sizeof(float)];
            std::memcpy(tmp, &f, sizeof(float));
            std::reverse(tmp, tmp + sizeof(float));
            std::memcpy(dst, tmp, sizeof(float));
        } else {
            std::memcpy(dst, &f, sizeof(float));
        }
        break;
    }
    case FieldType::Float64: {
        double d = value.toDouble();
        if (bigEndian) {
            char tmp[sizeof(double)];
            std::memcpy(tmp, &d, sizeof(double));
            std::reverse(tmp, tmp + sizeof(double));
            std::memcpy(dst, tmp, sizeof(double));
        } else {
            std::memcpy(dst, &d, sizeof(double));
        }
        break;
    }
    case FieldType::Bool: {
        const quint8 v = value.toBool() ? 1 : 0;
        std::memcpy(dst, &v, 1);
        break;
    }
    case FieldType::Bytes: {
        const QByteArray bytes = value.toByteArray();
        const int copyLen = qMin(bytes.size(), field.length);
        if (copyLen > 0) {
            std::memcpy(dst, bytes.constData(), copyLen);
        }
        break;
    }
    }
}

// ============================================================
// 内部校验和计算
// ============================================================

/**
 * @brief 计算指定数据的校验和
 *
 * 支持 Sum8/Xor8/CRC-8/CRC-16(MODBUS)/CRC-32 五种校验算法。
 */
QByteArray SerialFrameDecoder::computeChecksum(const QByteArray &data,
                                                 ChecksumType type,
                                                 int length) const
{
    QByteArray result(length, 0x00);

    switch (type) {
    case ChecksumType::None:
        break;

    case ChecksumType::Sum8: {
        quint8 sum = 0;
        for (char b : data) {
            sum += static_cast<quint8>(b);
        }
        result[0] = static_cast<char>(sum);
        break;
    }

    case ChecksumType::Xor8: {
        quint8 xval = 0;
        for (char b : data) {
            xval ^= static_cast<quint8>(b);
        }
        result[0] = static_cast<char>(xval);
        break;
    }

    case ChecksumType::Crc8: {
        // CRC-8/MAXIM 多项式 0x07
        quint8 crc = 0x00;
        for (char b : data) {
            crc ^= static_cast<quint8>(b);
            for (int i = 0; i < 8; ++i) {
                crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
            }
        }
        result[0] = static_cast<char>(crc);
        break;
    }

    case ChecksumType::Crc16: {
        // CRC-16/MODBUS: 初值 0xFFFF, 多项式 0xA001
        quint16 crc = 0xFFFF;
        for (char b : data) {
            crc ^= static_cast<quint8>(b);
            for (int i = 0; i < 8; ++i) {
                crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
            }
        }
        result[0] = static_cast<char>(crc & 0xFF);
        result[1] = static_cast<char>((crc >> 8) & 0xFF);
        break;
    }

    case ChecksumType::Crc32: {
        // CRC-32/ISO-HDLC: 初值 0xFFFFFFFF, 多项式 0xEDB88320 (反转)
        quint32 crc = 0xFFFFFFFF;
        for (char b : data) {
            crc ^= static_cast<quint8>(b);
            for (int i = 0; i < 8; ++i) {
                crc = (crc & 0x00000001) ? (crc >> 1) ^ 0xEDB88320u : (crc >> 1);
            }
        }
        crc = ~crc;
        result[0] = static_cast<char>((crc) & 0xFF);
        result[1] = static_cast<char>((crc >> 8) & 0xFF);
        result[2] = static_cast<char>((crc >> 16) & 0xFF);
        result[3] = static_cast<char>((crc >> 24) & 0xFF);
        break;
    }
    }

    return result;
}

/**
 * @brief 获取校验和覆盖的数据范围
 *
 * 默认范围: [headerSize, checksumOffset) — 即帧头之后到校验和之前的所有数据。
 * 当 checksumOffset < 0 时，校验整个帧体(去掉帧头帧尾)。
 */
QPair<int, int> SerialFrameDecoder::checksumRange(int frameSize) const
{
    const int headerSize = m_frameDef.header.size();
    const int csOffset = m_frameDef.checksumOffset;

    if (csOffset < 0) {
        // 无明确偏移时，校验整个帧体(去掉帧头帧尾)
        const int footerSize = m_frameDef.footer.size();
        return {headerSize, frameSize - footerSize};
    }

    return {headerSize, csOffset};
}
