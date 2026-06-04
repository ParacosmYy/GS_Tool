/**
 * @file SignalGeneratorWidgetStats.cpp
 * @brief 信号发生器统计查询与重置方法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 SignalGeneratorWidget.cpp 拆分而来，包含 stats() 和 resetStatistics()。
 */

#include "utils/signal_gen/SignalGeneratorWidget.h"

/** @brief 获取统计信息快照 @return 当前统计数据的只读副本 */
SignalGeneratorWidget::Stats SignalGeneratorWidget::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器(生成次数/采样点数/字节数/输出次数/错误次数/峰值归零) */
void SignalGeneratorWidget::resetStatistics()
{
    m_stats.totalGenerations = 0;
    m_stats.totalSamplesGenerated = 0;
    m_stats.totalBytesOutput = 0;
    m_stats.totalOutputs = 0;
    m_stats.errorCount = 0;
    m_stats.peakSamplesPerGen = 0;
}
