/**
 * @file ProtobufDecoderField.cpp
 * @brief Protobuf wire type字段解码实现 - 支持嵌套递归与重复字段跟踪
 *
 * 从 ProtobufDecoderVarint.cpp 拆分而来，包含:
 *   - decodeField(): 根据wire type解码单个Protobuf字段
 *     支持 wire type 0(varint)/1(64-bit)/2(length-delimited,含嵌套)/5(32-bit)
 *   - 字段出现次数跟踪(fieldTracker)用于重复字段聚合
 *
 * varint编解码实现见 ProtobufDecoderVarint.cpp。
 */

#include "protocol/protobuf/ProtobufDecoder.h"

/** @brief 从二进制数据中解码单个字段（含嵌套递归+重复字段跟踪）
 *
 * 解码流程:
 *   1. 调用parseVarint解析tag → 提取field_number和wire_type
 *   2. 根据wire_type分发到对应解码逻辑:
 *      - wire type 0 (Varint): 解析varint值
 *      - wire type 1 (64-bit): 读取8字节小端序
 *      - wire type 2 (Length-delimited): 解析长度后读取字节，
 *        若启用DecodeNested则递归尝试子消息解码
 *      - wire type 5 (32-bit): 读取4字节小端序
 *   3. 跟踪字段出现次数用于重复字段聚合
 *
 * @param data          源二进制数据
 * @param offset        起始偏移量
 * @param depth         当前嵌套深度
 * @param fieldTracker  字段号→出现次数跟踪器
 * @return QPair<字段信息Map, 新偏移量>，解析失败时返回空Map和原偏移量
 */
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
