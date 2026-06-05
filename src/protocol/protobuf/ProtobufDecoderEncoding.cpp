/**
 * @file ProtobufDecoderEncoding.cpp
 * @brief Protobuf编码接口与统计接口实现
 *
 * 从ProtobufDecoder.cpp拆分而来，包含:
 *   - encodeMessage: 将字段映射编码为Protobuf二进制
 *   - 全部统计getter / reset接口
 *
 * varint编解码和wire type字段解码见 ProtobufDecoderVarint.cpp
 */

#include "protocol/protobuf/ProtobufDecoder.h"

#include <QBuffer>

// ============================================================================
// 编码接口
// ============================================================================

/** @brief 编码为Protobuf二进制消息(支持嵌套编码)
 *
 * 遍历字段映射表，对每个字段:
 *   1. 解析字段号为整数
 *   2. 从字段Map中提取wireType和value
 *   3. 编码tag (field_number << 3 | wire_type) 为varint
 *   4. 调用encodeSingleValue编码字段值
 * 支持QVariantList编码重复字段
 *
 * @param fields 字段映射表，key为字段号字符串，value为字段信息Map或QVariantList
 * @return 编码后的二进制数据
 */
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

                quint64 tag = (static_cast<quint64>(fieldNum) << 3) | static_cast<quint64>(wireType);
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
