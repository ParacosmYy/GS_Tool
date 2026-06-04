/**
 * @file WaveformPreviewWidgetStats.cpp
 * @brief 波形预览控件统计接口实现
 *
 * 包含 resetStatistics() 统计重置方法。
 */

#include "chart/preview/WaveformPreviewWidget.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * 将 Stats 结构体值初始化归零，保留 hasValue=false
 * 以便下次首次数据点正确初始化 peakValue/minValue。
 */
void WaveformPreviewWidget::resetStatistics()
{
    m_stats = Stats{};
}
