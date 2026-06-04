/**
 * @file ProtobufDecoderHelpers.cpp
 * @brief Protobuf解码器辅助函数实现
 *
 * 从 ProtobufDecoder.cpp 拆分出的辅助功能:
 *   - tryDecodeNested: 嵌套消息启发式检测与递归解码
 *   - encodeSingleValue: 单值编码辅助（支持嵌套消息递归编码）
 *
 * 这些函数由 ProtobufDecoder 内部调用，不对外暴露。
 */

#include "protocol/protobuf/ProtobufDecoder.h"

#include <QBuffer>
#include <QVariantList>

// ============================================================================
// 嵌套消息启发式检测
// ============================================================================

/** @brief 尝试将length-delimited数据解码为嵌套消息 @param data 字节数据 @param depth 当前嵌套深度 @param success 是否成功解码 @return 解码结果(QVariantMap或QByteArray) */
QVariant ProtobufDecoder::tryDecodeNested(
    const QByteArray& data, int depth, bool& success) const
{
    success = false;

    // 启发式判断: 有效子消息应满足:
    //   1. 非空且至少2字节（1字节tag + 1字节值）
    //   2. 第一个varint解析出的tag字段号在有效范围[1, 536870911]
    //   3. wire type在有效范围[0, 5]（排除3,4已废弃）
    //   4. 能完整消费所有字节

    if (data.size() < 2) {
        return QVariant();
    }

    // 快速检查: 纯ASCII文本不太可能是子消息
    int printableCount = 0;
    for (int i = 0; i < qMin(data.size(), 16); ++i) {
        unsigned char ch = static_cast<unsigned char>(data[i]);
        if (ch >= 0x20 && ch < 0x7F) {
            ++printableCount;
        }
    }
    if (printableCount == qMin(data.size(), 16) && data.size() >= 4) {
        // 全是可打印字符，大概率是字符串而不是嵌套消息
        return QVariant();
    }

    // 尝试解析第一个tag
    auto [firstTag, firstTagBytes] = parseVarint(data, 0);
    if (firstTagBytes == 0) {
        return QVariant();
    }

    int fieldNum = static_cast<int>(firstTag >> 3);
    int wireType = static_cast<int>(firstTag & 0x07);

    // 字段号必须在有效范围 [1, 2^29 - 1]
    if (fieldNum < 1 || fieldNum > 536870911) {
        return QVariant();
    }

    // wire type 必须是 0, 1, 2, 5 之一
    if (wireType != 0 && wireType != 1 && wireType != 2 && wireType != 5) {
        return QVariant();
    }

    // 尝试完整解码
    QMap<int, int> subTracker;
    QVariantMap nestedResult;
    int offset = 0;
    int fieldCount = 0;

    while (offset < data.size()) {
        auto [fieldInfo, newOffset] = decodeField(data, offset, depth, subTracker);
        if (newOffset <= offset) {
            // 解析失败，不是有效子消息
            return QVariant();
        }
        offset = newOffset;
        ++fieldCount;

        int fn = fieldInfo.value("fieldNumber").toInt();
        QString key = QString::number(fn);

        // 重复字段聚合（嵌套内）
        if (m_decodeOptions.testFlag(CollectRepeated) &&
            subTracker.value(fn, 0) > 1) {
            QVariantList list;
            if (nestedResult.contains(key) &&
                nestedResult[key].canConvert<QVariantList>()) {
                list = nestedResult[key].toList();
            } else if (nestedResult.contains(key)) {
                list.append(nestedResult[key]);
            }
            list.append(fieldInfo);
            nestedResult[key] = QVariant::fromValue(list);
        } else {
            nestedResult[key] = fieldInfo;
        }
    }

    // 必须完整消费所有字节且至少解析出一个字段
    if (offset == data.size() && fieldCount > 0) {
        success = true;
        ++m_stats.totalNestedMessages;
        if (static_cast<quint64>(depth) > m_stats.maxNestingDepth) {
            m_stats.maxNestingDepth = static_cast<quint64>(depth);
        }
        return QVariant::fromValue(nestedResult);
    }

    return QVariant();
}

// ============================================================================
// 编码辅助
// ============================================================================

/** @brief 编码单个值到缓冲区 @param buf 目标缓冲区 @param wireType wire类型 @param val 值 */
void ProtobufDecoder::encodeSingleValue(
    QBuffer& buf, int wireType, const QVariant& val) const
{
    switch (wireType) {
    case 0: { // Varint
        quint64 v = val.toULongLong();
        writeVarint(buf, v);
        break;
    }
    case 1: { // 64-bit fixed
        quint64 v = val.toULongLong();
        for (int i = 0; i < 8; ++i) {
            buf.putChar(static_cast<char>((v >> (i * 8)) & 0xFF));
        }
        break;
    }
    case 2: { // Length-delimited
        QByteArray bytes;
        if (val.typeId() == QMetaType::QByteArray) {
            bytes = val.toByteArray();
        } else if (val.typeId() == QMetaType::QVariantMap) {
            // 嵌套消息编码: 递归编码子消息
            bytes = encodeMessage(val.toMap());
        } else {
            bytes = val.toString().toUtf8();
        }
        writeVarint(buf, static_cast<quint64>(bytes.size()));
        buf.write(bytes);
        break;
    }
    case 5: { // 32-bit fixed
        quint32 v = val.toUInt();
        for (int i = 0; i < 4; ++i) {
            buf.putChar(static_cast<char>((v >> (i * 8)) & 0xFF));
        }
        break;
    }
    default:
        break;
    }
}
