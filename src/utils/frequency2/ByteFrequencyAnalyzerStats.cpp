/**
 * @file ByteFrequencyAnalyzerStats.cpp
 * @brief 字节频率分析器 -- 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 stats() 和 resetStatistics() 方法实现，
 * 从 ByteFrequencyAnalyzer.cpp 拆分而来。
 */

#include "utils/frequency2/ByteFrequencyAnalyzer.h"

/** @brief 获取累计统计数据快照 @return Stats 结构体副本 */
ByteFrequencyAnalyzer::Stats ByteFrequencyAnalyzer::stats() const
{
    return m_stats;
}

/** @brief 重置所有累计统计计数器和分析状态
 *
 * 清零: 分析次数、累计字节数、检测模式数、平均熵、最大单次字节数。
 * 同时清空 256 级计数数组、缓存数据和最近分析结果，
 * 恢复为构造后的初始状态。
 */
void ByteFrequencyAnalyzer::resetStatistics()
{
    /* 累计统计归零 */
    m_stats = Stats{};

    /* 字节计数归零 */
    std::fill(m_byteCounts.begin(), m_byteCounts.end(), quint64(0));

    /* 清空缓存数据和分析结果 */
    m_lastData.clear();
    m_lastResult = AnalysisResult{};
}
