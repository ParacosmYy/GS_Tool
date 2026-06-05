/**
 * @file AnnotationManagerStats.cpp
 * @brief 数据标注管理器统计方法实现
 */

#include "utils/annotation/AnnotationManager.h"

/** @brief 重置所有统计计数器（保留 activeCount 和 peakCount 的瞬时值） */
void AnnotationManager::resetStatistics()
{
    int active = m_stats.activeCount;
    int peak   = m_stats.peakCount;
    m_stats = Stats{};
    m_stats.activeCount = active;
    m_stats.peakCount   = peak;
}
