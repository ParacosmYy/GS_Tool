/**
 * @file SchemaViewerStats.cpp
 * @brief 模式查看器 - 解码统计与重置接口实现
 *
 * 从 SchemaViewer.cpp 拆分而来，包含记录解码/解析错误、
 * 吞吐量快照、所有统计getter和resetStatistics方法。
 */

#include "protocol/protobuf/SchemaViewer.h"

/** @brief 记录一次解码消息(更新吞吐量窗口) @param fieldCount 本消息的字段数 */
void SchemaViewer::recordDecodedMessage(int fieldCount)
{
    ++m_decodedMessages;
    m_decodedFields += static_cast<quint64>(qMax(fieldCount, 0));

    // 更新吞吐量滑动窗口
    ++m_throughputMsgCount;
    m_throughputFieldCount += static_cast<quint64>(qMax(fieldCount, 0));

    // 每隔5秒重置窗口避免速率失真
    if (m_throughputTimer.elapsed() > 5000) {
        m_throughputMsgCount = 0;
        m_throughputFieldCount = 0;
        m_throughputTimer.restart();
    }
}

/** @brief 记录一次解析错误 */
void SchemaViewer::recordParseError()
{
    ++m_parseErrors;
}

/** @brief 获取桥接吞吐量快照 @return BridgeThroughput统计 */
SchemaViewer::BridgeThroughput SchemaViewer::bridgeThroughput() const
{
    qint64 elapsedMs = m_throughputTimer.elapsed();
    if (elapsedMs <= 0) {
        return m_cachedThroughput;
    }

    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;
    if (elapsedSec < 0.5) {
        return m_cachedThroughput;
    }

    m_cachedThroughput.decodedMessages = m_decodedMessages;
    m_cachedThroughput.decodedFields = m_decodedFields;
    m_cachedThroughput.parseErrors = m_parseErrors;
    m_cachedThroughput.messagesPerSec =
        static_cast<double>(m_throughputMsgCount) / elapsedSec;
    m_cachedThroughput.fieldsPerSec =
        static_cast<double>(m_throughputFieldCount) / elapsedSec;

    return m_cachedThroughput;
}

/** @brief 获取累计加载Schema次数 @return 加载次数 */
quint64 SchemaViewer::totalSchemasLoaded() const
{
    return m_totalSchemasLoaded;
}

/** @brief 获取累计字段展开次数 @return 展开次数 */
quint64 SchemaViewer::totalFieldExpansions() const
{
    return m_totalFieldExpansions;
}

/** @brief 获取累计解码消息总数 @return 消息数 */
quint64 SchemaViewer::totalDecodedMessages() const
{
    return m_decodedMessages;
}

/** @brief 获取累计解码字段总数 @return 字段数 */
quint64 SchemaViewer::totalDecodedFields() const
{
    return m_decodedFields;
}

/** @brief 获取累计解析错误总数 @return 错误数 */
quint64 SchemaViewer::totalParseErrors() const
{
    return m_parseErrors;
}

/** @brief 重置所有统计计数器 */
void SchemaViewer::resetStatistics()
{
    m_totalSchemasLoaded = 0;
    m_totalFieldExpansions = 0;

    m_decodedMessages = 0;
    m_decodedFields = 0;
    m_parseErrors = 0;

    m_throughputMsgCount = 0;
    m_throughputFieldCount = 0;
    m_throughputTimer.restart();
    m_cachedThroughput = BridgeThroughput();
}
