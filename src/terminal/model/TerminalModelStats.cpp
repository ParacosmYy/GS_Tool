/**
 * @file TerminalModelStats.cpp
 * @brief 终端数据模型 - 统计接口实现
 *
 * 从 TerminalModel.cpp 拆分而来，包含所有统计getter和resetStats方法。
 */

#include "terminal/model/TerminalModel.h"

/** @brief 获取累计追加行数(含被环形缓冲区覆盖的) @return 行数 */
quint64 TerminalModel::totalLinesAdded() const { return m_totalLinesAdded; }

/** @brief 获取累计接收字节数 @return 字节数 */
quint64 TerminalModel::totalBytesReceived() const { return m_totalBytesReceived; }

/** @brief 获取累计发送字节数 @return 字节数 */
quint64 TerminalModel::totalBytesSent() const { return m_totalBytesSent; }

/** @brief 获取历史最长行长度(字节数) @return 最大行长度 */
quint64 TerminalModel::maxLineLength() const { return m_maxLineLength; }

/** @brief 获取被过滤丢弃的行数 @return 过滤丢弃行数 */
quint64 TerminalModel::filterBlockCount() const { return m_filterBlockCount; }

/** @brief 获取环形缓冲区满覆盖次数 @return 覆盖次数 */
quint64 TerminalModel::totalMaxLinesReached() const { return m_totalMaxLinesReached; }

/** @brief 获取数据清空总次数 @return 清空次数 */
quint64 TerminalModel::totalClears() const { return m_totalClears; }

/** @brief 获取搜索执行总次数 @return 搜索次数 */
quint64 TerminalModel::totalSearchesPerformed() const { return m_totalSearchesPerformed; }

/** @brief 获取历史峰值行数 @return 峰值行数 */
quint64 TerminalModel::peakLineCount() const { return m_peakLineCount; }

/** @brief 通知搜索操作已执行，递增搜索计数 */
void TerminalModel::notifySearchPerformed() { ++m_totalSearchesPerformed; }

/** @brief 重置所有统计计数器为零 */
void TerminalModel::resetStats()
{
    m_totalLinesAdded = 0;
    m_totalBytesReceived = 0;
    m_totalBytesSent = 0;
    m_maxLineLength = 0;
    m_filterBlockCount = 0;
    m_totalMaxLinesReached = 0;
    m_totalClears = 0;
    m_totalSearchesPerformed = 0;
    m_peakLineCount = 0;
}
