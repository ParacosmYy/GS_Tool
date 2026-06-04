/**
 * @file ProtobufDecoderEncoding.cpp
 * @brief Protobuf编解码核心实现 — varint编解码、字段解码、编码和统计接口
 *
 * 从ProtobufDecoder.cpp拆分而来，包含:
 *   - writeVarint / parseVarint: varint编解码（最多10字节溢出保护）
 *   - encodeMessage: 将字段映射编码为Protobuf二进制
 *   - decodeField: 单字段解码（支持嵌套递归+重复字段跟踪）
 *   - 全部统计getter / reset接口
 */

#include "protocol/protobuf/ProtobufDecoder.h"

#include <QBuffer>

// ============================================================================
// Varint 最大字节数常量
// ============================================================================

/** @brief Protobuf varint最多占用10字节（64位最多9字节+1字节溢出保护） */
static constexpr int kMaxVarintBytes = 10;

// ============================================================================
// 编码接口
// ============================================================================

/** @brief 编码为Protobuf二进制消息(支持嵌套编码) @param fields 字段映射表 @return 编码后的二进制数据 */
QByteArray ProtobufDecoder::encodeMessage(const QVariantMap& fields) const {
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);

    for (auto it = fields.begin(); it != fields.end(); ++it) {
        bool ok = false;
        int fieldNum = it.key().toInt(&ok);
        if (!ok) { continue; }

        QVariant fieldVal = it.value();

        // 支持QVariantList编码(重复字段)
        if (fieldVal.typeId() == QMetaType::QVariantList) {
            QVariantList list = fieldVal.toList();
            for (const QVariant& item : list) {
                QVariantMap fieldMap = item.toMap();
                int wireType = fieldMap.value("wireType", 0).toInt();
                QVariant val = fieldMap.value("value");

                quint64 tag = static_cast<quint64>((fieldNum << 3) | wireType);
                writeVarint(buf, tag);
                encodeSingleValue(buf, wireType, val);
            }
            continue;
        }

        QVariantMap fieldMap = fieldVal.toMap();
        int wireType = fieldMap.value("wireType", 0).toInt();
        QVariant val = fieldMap.value("value");

        quint64 tag = static_cast<quint64>((fieldNum << 3) | wireType);
        writeVarint(buf, tag);
        encodeSingleValue(buf, wireType, val);
    }

    buf.close();
    return data;
}

// ============================================================================
// Varint编解码
// ============================================================================

/** @brief 将varint编码写入缓冲区 @param buf 目标缓冲区 @param value 待写入的64位无符号整数 */
void ProtobufDecoder::writeVarint(QBuffer& buf, quint64 value) const {
    while (value > 0x7F) {
        buf.putChar(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    buf.putChar(static_cast<char>(value & 0x7F));
}

/** @brief 从数据中解析一个varint值（带溢出保护，最多10字节） @param data 源二进制数据 @param offset 起始偏移量 @return QPair<解析后的值,消耗的字节数>，出错时返回{0, 0} */
QPair<quint64, int> ProtobufDecoder::parseVarint(const QByteArray& data,
                                                  int offset) const {
    quint64 value = 0;
    int shift = 0;
    int pos = offset;

    while (pos < data.size()) {
        // 溢出保护: varint最多10字节
        if (pos - offset >= kMaxVarintBytes) {
            // 超过最大长度，数据损坏
            return {0, 0};
        }

        quint8 byte = static_cast<quint8>(data[pos]);
        value |= static_cast<quint64>(byte & 0x7F) << shift;
        pos++;

        // 最高位为0表示varint结束
        if ((byte & 0x80) == 0) {
            return {value, pos - offset};
        }

        shift += 7;

        // 位移超过63位（64位无符号整数最大范围）的额外保护
        if (shift >= 64) {
            // 第10个字节只允许最低位有效（Protobuf规范）
            if (pos - offset == 10) {
                quint8 lastByte = static_cast<quint8>(data[pos - 1]);
                if ((lastByte & 0x7E) != 0) {
                    // 超过64位范围，数据损坏
                    return {0, 0};
                }
                value |= static_cast<quint64>(lastByte & 0x01) << 63;
                return {value, pos - offset};
            }
            return {0, 0};
        }
    }

    // 到达数据末尾但varint未结束
    return {0, 0};
}

// ============================================================================
// 字段解码（支持嵌套和重复）
// ============================================================================

/** @brief 从二进制数据中解码单个字段（含嵌套递归+重复字段跟踪） @param data 源二进制数据 @param offset 起始偏移量 @param depth 当前嵌套深度 @param fieldTracker 字段号→出现次数跟踪器 @return QPair<字段信息Map,新偏移量> */
QPair<QVariantMap, int> ProtobufDecoder::decodeField(
    const QByteArray& data, int offset, int depth,
    QMap<int, int>& fieldTracker) const
{
    QVariantMap field;
    int consumed = 0;

    // 解析tag (field_number << 3 | wire_type)
    auto [tag, tagBytes] = parseVarint(data, offset);
    if (tagBytes == 0) {
        // varint解析失败
        return {field, offset};
    }
    consumed += tagBytes;

    int fieldNumber = static_cast<int>(tag >> 3);
    int wireType    = static_cast<int>(tag & 0x07);

    field["fieldNumber"] = fieldNumber;
    field["wireType"]    = wireType;
    field["nestingDepth"] = depth;

    // 跟踪字段出现次数
    fieldTracker[fieldNumber] = fieldTracker.value(fieldNumber, 0) + 1;

    // 根据wire type解析值
    switch (wireType) {
    case 0: { // Varint
        auto [val, vb] = parseVarint(data, offset + consumed);
        if (vb == 0) {
            return {field, offset};
        }
        consumed += vb;
        field["value"] = QVariant::fromValue(val);
        break;
    }
    case 1: { // 64-bit
        if (offset + consumed + 8 > data.size()) {
            return {field, offset};
        }
        quint64 val = 0;
        for (int i = 0; i < 8; ++i) {
            val |= static_cast<quint64>(
                static_cast<quint8>(data[offset + consumed + i]))
                << (i * 8);
        }
        consumed += 8;
        field["value"] = QVariant::fromValue(val);
        break;
    }
    case 2: { // Length-delimited
        auto [len, lb] = parseVarint(data, offset + consumed);
        if (lb == 0) {
            return {field, offset};
        }
        consumed += lb;
        if (offset + consumed + static_cast<int>(len) > data.size()) {
            return {field, offset};
        }
        QByteArray bytes = data.mid(offset + consumed,
                                     static_cast<int>(len));
        consumed += static_cast<int>(len);

        // 尝试嵌套消息解码（如果启用且未超出深度限制）
        if (m_decodeOptions.testFlag(DecodeNested) &&
            depth < m_maxNestingDepth) {
            bool nestedOk = false;
            QVariant nestedResult = tryDecodeNested(bytes, depth + 1, nestedOk);
            if (nestedOk) {
                field["value"] = nestedResult;
                field["isNested"] = true;
            } else {
                // 不是有效子消息，保留原始字节
                field["value"] = bytes;
            }
        } else {
            field["value"] = bytes;
        }
        break;
    }
    case 5: { // 32-bit
        if (offset + consumed + 4 > data.size()) {
            return {field, offset};
        }
        quint32 val = 0;
        for (int i = 0; i < 4; ++i) {
            val |= static_cast<quint32>(
                static_cast<quint8>(data[offset + consumed + i]))
                << (i * 8);
        }
        consumed += 4;
        field["value"] = QVariant::fromValue(val);
        break;
    }
    default:
        // 未知wire type，无法继续解析
        return {field, offset};
    }

    return {field, offset + consumed};
}

// ============================================================================
// 统计接口实现
// ============================================================================

/** @brief 获取累计解码的消息总数 @return 解码总数 */
quint64 ProtobufDecoder::totalDecoded() const
{
    return m_stats.totalMessages;
}

/** @brief 获取累计解码的字节总数 @return 字节总数 */
quint64 ProtobufDecoder::totalBytesDecoded() const
{
    return m_stats.totalBytes;
}

/** @brief 获取累计解码错误次数 @return 错误次数 */
quint64 ProtobufDecoder::errorCount() const
{
    return m_stats.totalErrors;
}

/** @brief 获取累计解码字段总数(含嵌套字段) @return 字段总数 */
quint64 ProtobufDecoder::totalFieldsDecoded() const
{
    return m_stats.totalFields;
}

/** @brief 获取累计嵌套消息解码次数 @return 嵌套解码次数 */
quint64 ProtobufDecoder::totalNestedDecoded() const
{
    return m_stats.totalNestedMessages;
}

/** @brief 获取累计重复字段聚合次数 @return 聚合次数 */
quint64 ProtobufDecoder::totalRepeatedAggregated() const
{
    return m_stats.totalRepeatedFields;
}

/** @brief 获取历史最大嵌套深度 @return 最大深度 */
quint64 ProtobufDecoder::maxObservedNestingDepth() const
{
    return m_stats.maxNestingDepth;
}

/** @brief 获取完整统计快照 @return DecodeStatistics结构体 */
ProtobufDecoder::DecodeStatistics ProtobufDecoder::statistics() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void ProtobufDecoder::resetDecoderStatistics()
{
    m_stats = DecodeStatistics();
}
