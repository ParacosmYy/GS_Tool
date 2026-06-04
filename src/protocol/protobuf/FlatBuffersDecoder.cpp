/**
 * @file FlatBuffersDecoder.cpp
 * @brief FlatBuffers解码器 - 顶层解码入口与统计接口
 *
 * 提供 decodeMessage() 顶层入口方法及统计 getter/setter。
 * 字段解码、类型解析、底层读取见 FlatBuffersDecoderFields.cpp。
 * Schema加载/解析逻辑见 FlatBuffersDecoderSchema.cpp。
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

// ───────────────────── 顶层解码入口 ─────────────────────

/**
 * @brief 解码FlatBuffers二进制消息
 *
 * 读取根偏移量定位根表，根据已加载的Schema定义解析各字段。
 * @param data FlatBuffers格式的二进制数据
 * @return 解析结果Map，key为字段名，value为字段值
 */
QVariantMap FlatBuffersDecoder::decodeMessage(const QByteArray& data) {
    if (data.size() < 8) {
        ++m_errorCount;
        return {};
    }
    m_totalBytesDecoded += static_cast<quint64>(data.size());
    quint32 rootOff = readOffset(data, 0);
    const FbsTableDef* root = findRootTable();
    QVariantMap result = parseTable(data, static_cast<int>(rootOff),
                      root ? root->name : QString());
    ++m_totalDecoded;
    return result;
}

// ── 统计接口实现 ──

/** @brief 获取累计解码的消息总数 @return 解码总数 */
quint64 FlatBuffersDecoder::totalDecoded() const
{
    return m_totalDecoded;
}

/** @brief 获取累计解码的字节总数 @return 字节总数 */
quint64 FlatBuffersDecoder::totalBytesDecoded() const
{
    return m_totalBytesDecoded;
}

/** @brief 获取累计解码错误次数 @return 错误次数 */
quint64 FlatBuffersDecoder::errorCount() const
{
    return m_errorCount;
}

/** @brief 获取累计解码的Table结构总数(含嵌套table) @return Table解码总数 */
quint64 FlatBuffersDecoder::totalTablesDecoded() const
{
    return m_totalTablesDecoded;
}

/** @brief 获取累计读取的字段总数(含嵌套字段) @return 字段读取总数 */
quint64 FlatBuffersDecoder::totalFieldsRead() const
{
    return m_totalFieldsRead;
}

/** @brief 重置所有统计计数器 */
void FlatBuffersDecoder::resetDecoderStatistics()
{
    m_totalDecoded = 0;
    m_totalBytesDecoded = 0;
    m_errorCount = 0;
    m_totalTablesDecoded = 0;
    m_totalFieldsRead = 0;
}
