/**
 * @file ScrollChartWidgetStats.cpp
 * @brief 轻量级滚动折线图 — 统计重置实现
 *
 * 从 ScrollChartWidget.cpp 拆分而来，专注运行统计计数器的重置逻辑。
 */

#include "widgets/chart/ScrollChartWidget.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * 清零: totalSamplesAdded / totalPaints / totalChannelChanges / totalExports
 *       peakSampleRate / maxChannels
 * 注意: 通道历史数据(minVal/maxVal)不受影响，仅重置累计计数器。
 */
void ScrollChartWidget::resetStatistics()
{
    m_stats = Stats{};
}
