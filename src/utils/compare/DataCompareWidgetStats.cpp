/**
 * @file DataCompareWidgetStats.cpp
 * @brief 数据对比控件统计查询和重置方法实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 从 DataCompareWidget.cpp 拆分而来，包含 stats() getter
 * 和 resetStatistics() 方法。
 */

#include "utils/compare/DataCompareWidget.h"

/** @brief 获取累计统计数据快照 @return 包含对比次数/字节数/差异数/导航次数/复制次数/相似度极值的结构体 */
DataCompareWidget::Stats DataCompareWidget::stats() const
{
    return m_stats;
}

/** @brief 重置所有累计统计计数器为初始值(对比次数/字节数/差异数/导航次数/复制次数归零，相似度极值重置) */
void DataCompareWidget::resetStatistics()
{
    m_stats.totalCompares = 0;
    m_stats.totalBytesCompared = 0;
    m_stats.totalDiffsFound = 0;
    m_stats.totalNavigations = 0;
    m_stats.totalCopies = 0;
    m_stats.peakSimilarity = 0.0;
    m_stats.lowestSimilarity = 100.0;
}
