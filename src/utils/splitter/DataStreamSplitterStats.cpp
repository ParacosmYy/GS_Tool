/**
 * @file DataStreamSplitterStats.cpp
 * @brief 数据流分割器统计查询和重置方法实现
 *
 * 从 DataStreamSplitter.cpp 拆分而来，包含统计信息查询和重置方法。
 */

#include "utils/splitter/DataStreamSplitter.h"

/** @brief 获取运行时统计信息 @return 统计数据的const引用 */
const DataStreamSplitter::Stats& DataStreamSplitter::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器为初始值
 *
 * 不影响当前分割规则、缓冲区内容和帧计数器。
 * 重置后avgFrameSize归零，下次分割时重新开始计算。
 */
void DataStreamSplitter::resetStatistics()
{
    m_stats.totalFramesSplit = 0;
    m_stats.totalBytesProcessed = 0;
    m_stats.totalDiscarded = 0;
    m_stats.currentRuleIndex = 0;
    m_stats.splitErrors = 0;
    m_stats.avgFrameSize = 0;
}
