/**
 * @file CircularBufferWidgetStats.cpp
 * @brief CircularBufferWidget 统计计数器重置实现
 *
 * 从 CircularBufferWidget.cpp 拆分而来，仅包含 resetStatistics() 方法。
 * 保留 peakUsed 不重置(反映当前真实峰值)，重置为当前使用量。
 */

#include "widgets/circular/CircularBufferWidget.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * peakUsed 重置为当前 m_used 值(保留当前状态作为新基准)，
 * cumulativePercent 归零(清除历史平均基准)，
 * 其余累计计数器归零。
 */
void CircularBufferWidget::resetStatistics()
{
    m_stats.totalUpdates = 0;
    m_stats.totalOverflows = 0;
    m_stats.totalCapacityChanges = 0;
    m_stats.totalPointerMoves = 0;
    m_stats.totalRenders = 0;
    m_stats.totalResets = 0;
    m_stats.peakUsed = m_used;             ///< 重置峰值为当前使用量
    m_stats.cumulativePercent = 0.0;       ///< 清除累计填充率
}
