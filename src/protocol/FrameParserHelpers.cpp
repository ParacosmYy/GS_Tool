/**
 * @file FrameParserHelpers.cpp
 * @brief 帧解析辅助方法实现 — 字段提取、校验计算、长度解析、Payload子路径处理
 *
 * 从 FrameParser.cpp 中拆分出的辅助方法，职责:
 *   1. extractFields  — 从完整帧数据中提取各字段值
 *   2. verifyChecksum — 计算并验证校验值
 *   3. computeChecksum — 根据校验类型计算校验字节
 *   4. parseLengthField — 解析帧内长度字段值
 *   5. handlePayloadWithLength  — 有长度字段时的Payload接收处理
 *   6. handlePayloadWithFooter  — 无长度字段有帧尾时的Payload接收处理
 *   7. handlePayloadMinimal     — 最简帧的Payload接收处理
 *
 * 这些方法都是 FrameParser 类的 private 方法，
 * 操作的是已接收完成的帧数据或Payload子路径逻辑，与状态机主循环(processByte)解耦。
 */

#include "FrameParser.h"
#include "utils/HexConverter.h"
#include <QDateTime>

// ============================================================================
// 字段提取与校验计算
// ============================================================================

/**
 * @brief 提取帧内各字段值
 * @param frameData 完整帧数据
 * @return 字段名->值的映射
 *
 * 提取 payload 区域后，逐字段调用 FieldDef::extractValue。
 * 元数据字段: _rawPayload, _rawFrame(HEX), _frameTime
 */
QVariantMap FrameParser::extractFields(const QByteArray& frameData) const
{
    QVariantMap result;

    // 提取payload区域（帧头之后，校验/帧尾之前）
    int payloadStart = m_def.header.size();
    if (m_def.lengthFieldOffset >= 0) {
        payloadStart = m_def.lengthFieldOffset + m_def.lengthFieldSize;
    }

    int payloadEnd = frameData.size();
    if (m_def.checksumOffset >= 0) {
        payloadEnd = m_def.checksumOffset;
    } else if (!m_def.footer.isEmpty()) {
        payloadEnd = frameData.size() - m_def.footer.size();
    }

    QByteArray payload = frameData.mid(payloadStart, payloadEnd - payloadStart);
    result["_rawPayload"] = payload;
    result["_rawFrame"] = HexConverter::toHexString(frameData);
    result["_frameTime"] = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    // 逐字段提取值
    for (const auto& field : m_def.fields) {
        QString key = field.name.isEmpty() ? QString("field_%1").arg(field.offset) : field.name;
        result[key] = field.formatValue(payload);
    }

    return result;
}

/**
 * @brief 计算并验证校验值
 * @param frameData 完整帧数据
 * @return true=校验通过，false=校验失败
 *
 * 从 checksumStart 到 checksumEnd 区域计算校验值，
 * 与帧内 checksumOffset 处的校验字段比较。
 */
bool FrameParser::verifyChecksum(const QByteArray& frameData) const
{
    if (m_def.checksumType == ChecksumType::None || m_def.checksumOffset < 0) {
        return true;
    }

    int start = m_def.checksumStart;
    int end = (m_def.checksumEnd < 0) ? m_def.checksumOffset : m_def.checksumEnd;

    if (start < 0 || end > frameData.size() || start >= end) {
        return false;
    }

    QByteArray checkRegion = frameData.mid(start, end - start);
    QByteArray computed = computeChecksum(checkRegion);
    QByteArray actual = frameData.mid(m_def.checksumOffset, m_def.checksumSize);

    return (computed == actual);
}

/**
 * @brief 计算校验值
 * @param data 需要计算校验的数据区域
 * @return 校验结果字节数组
 *
 * 根据 checksumType 选择对应的校验算法。
 */
QByteArray FrameParser::computeChecksum(const QByteArray& data) const
{
    QByteArray result;
    switch (m_def.checksumType) {
    case ChecksumType::None:
        break;
    case ChecksumType::Sum8:
        result.append(static_cast<char>(CRC::checksum(data)));
        break;
    case ChecksumType::CRC8:
        result.append(static_cast<char>(CRC::crc8(data)));
        break;
    case ChecksumType::CRC16CCITT: {
        uint16_t crc = CRC::crc16Ccitt(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        break;
    }
    case ChecksumType::CRC16Modbus: {
        uint16_t crc = CRC::crc16Modbus(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        break;
    }
    case ChecksumType::CRC32: {
        uint32_t crc = CRC::crc32(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        result.append(static_cast<char>((crc >> 16) & 0xFF));
        result.append(static_cast<char>((crc >> 24) & 0xFF));
        break;
    }
    }
    return result;
}

/**
 * @brief 解析长度字段值
 * @param frameData 包含长度字段的帧数据
 * @return 解析出的长度值，-1 表示解析失败
 *
 * 支持 1 字节和 2 字节长度字段，大小端由 lengthBigEndian 决定。
 * 返回值 = 原始长度值 - lengthAdjust
 */
int FrameParser::parseLengthField(const QByteArray& frameData) const
{
    if (m_def.lengthFieldOffset < 0) return -1;

    int offset = m_def.lengthFieldOffset;
    if (offset + m_def.lengthFieldSize > frameData.size()) return -1;

    int length = 0;
    if (m_def.lengthFieldSize == 1) {
        length = static_cast<unsigned char>(frameData.at(offset));
    } else if (m_def.lengthFieldSize == 2) {
        if (m_def.lengthBigEndian) {
            length = (static_cast<unsigned char>(frameData.at(offset)) << 8) |
                     static_cast<unsigned char>(frameData.at(offset + 1));
        } else {
            length = static_cast<unsigned char>(frameData.at(offset)) |
                     (static_cast<unsigned char>(frameData.at(offset + 1)) << 8);
        }
    }

    return length - m_def.lengthAdjust;
}
