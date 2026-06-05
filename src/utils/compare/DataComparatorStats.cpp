/**
 * @file DataComparatorStats.cpp
 * @brief 数据比较引擎统计查询和重置方法实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 从 DataComparator.cpp 拆分而来，包含 stats() getter
 * 和 resetStatistics() 方法。
 */

#include "utils/compare/DataComparator.h"

/** @brief 获取累计统计计数器快照 @return 包含比较次数/各类字节数/相似度/数据长度极值的结构体 */
DataComparator::Stats DataComparator::stats() const
{
    return m_stats;
}

/** @brief 重置所有累计统计计数器为初始值（比较次数/字节数/相似度总和/数据长度极值全部归零） */
void DataComparator::resetStatistics()
{
    m_stats.totalComparisons = 0;
    m_stats.totalEqualBytes = 0;
    m_stats.totalDifferentBytes = 0;
    m_stats.totalInsertedBytes = 0;
    m_stats.totalDeletedBytes = 0;
    m_stats.avgSimilarity = 0;
    m_stats.minDataSize = 0;
    m_stats.maxDataSize = 0;
    m_similaritySum = 0.0;
}
