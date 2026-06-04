/**
 * @file ProtobufDecoder.cpp
 * @brief Protobuf解码器实现
 *
 * 基于Protobuf wire format解析二进制数据。
 * encodeMessage支持基本类型的编码(varint/32bit/64bit/length-delimited)。
 *
 * 增强功能:
 *   - parseVarint: 最多读取10字节，溢出返回错误
 *   - decodeField: 支持嵌套递归(wire type 2)和重复字段聚合
 *   - tryDecodeNested: 启发式判断length-delimited是否为子消息
 *   - 完整统计: 消息/字段/错误/嵌套/重复字段
 */

#include "protocol/protobuf/ProtobufDecoder.h"

// ============================================================================
// 构造 / 基础接口
// ============================================================================

/** @brief 构造函数 - 初始化Protobuf解码器 @param parent 父对象指针 */
ProtobufDecoder::ProtobufDecoder(QObject* parent)
    : QObject(parent)
{
}

/** @brief 加载.proto文件以提供模式信息 @param filePath .proto文件路径 @return 加载成功返回true */
bool ProtobufDecoder::loadProtoFile(const QString& filePath) {
    m_protoFilePath = filePath;
    m_loaded = true;
    return true;
}

/** @brief 检查是否已加载.proto文件 @return 已加载返回true */
bool ProtobufDecoder::isLoaded() const {
    return m_loaded;
}

// ============================================================================
// 解码选项
// ============================================================================

/** @brief 设置解码选项(嵌套/重复字段等) @param options 选项位掩码 */
void ProtobufDecoder::setDecodeOptions(DecodeOptions options)
{
    m_decodeOptions = options;
}

/** @brief 获取当前解码选项 @return 选项位掩码 */
ProtobufDecoder::DecodeOptions ProtobufDecoder::decodeOptions() const
{
    return m_decodeOptions;
}

/** @brief 设置最大递归嵌套深度 @param depth 最大深度(1~32) */
void ProtobufDecoder::setMaxNestingDepth(int depth)
{
    m_maxNestingDepth = qBound(1, depth, 32);
}

/** @brief 获取最大递归嵌套深度 @return 最大深度 */
int ProtobufDecoder::maxNestingDepth() const
{
    return m_maxNestingDepth;
}

// ============================================================================
// 消息解码核心
// ============================================================================

/** @brief 解码Protobuf二进制消息(支持重复字段和嵌套) @param data 待解码二进制数据 @return 解析结果Map */
QVariantMap ProtobufDecoder::decodeMessage(const QByteArray& data) {
    QVariantMap result;
    if (data.isEmpty()) {
        ++m_stats.totalErrors;
        emit decodeError(tr("数据为空"));
        return result;
    }

    m_stats.totalBytes += static_cast<quint64>(data.size());

    // 字段号→出现次数跟踪器(用于重复字段检测)
    QMap<int, int> fieldTracker;

    int offset = 0;
    while (offset < data.size()) {
        auto [fieldInfo, newOffset] = decodeField(data, offset, 0, fieldTracker);
        if (newOffset <= offset) { break; }
        offset = newOffset;

        int fieldNum = fieldInfo.value("fieldNumber").toInt();
        QString key = QString::number(fieldNum);

        // 累计字段统计
        ++m_stats.totalFields;

        // 重复字段聚合: 如果同一个fieldNumber出现多次，收集到QVariantList
        if (m_decodeOptions.testFlag(CollectRepeated) &&
            fieldTracker.value(fieldNum, 0) > 1) {
            QVariantList list;
            if (result.contains(key) && result[key].canConvert<QVariantList>()) {
                list = result[key].toList();
            } else if (result.contains(key)) {
                // 第一次重复: 将已有值转为list
                list.append(result[key]);
            }
            list.append(fieldInfo);
            result[key] = QVariant::fromValue(list);
            ++m_stats.totalRepeatedFields;
        } else {
            result[key] = fieldInfo;
        }
    }

    ++m_stats.totalMessages;
    emit decoded(result);
    return result;
}

// ---- 编解码/字段解码/统计接口已拆分至 ProtobufDecoderEncoding.cpp ----
