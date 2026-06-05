/**
 * @file SpectrumMonitorWidgetStats.cpp
 * @brief 频谱监控控件统计接口实现 — 计数器查询与重置
 *
 * 从 SpectrumMonitorWidget.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "chart/spectrum/SpectrumMonitorWidget.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计重绘次数 */
quint64 SpectrumMonitorWidget::totalRepaints() const
{
    return m_totalRepaints;
}

/** @brief 获取累计峰值保持切换次数 */
quint64 SpectrumMonitorWidget::totalPeakHoldToggles() const
{
    return m_totalPeakHoldToggles;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void SpectrumMonitorWidget::resetStatistics()
{
    m_totalRepaints       = 0;
    m_totalPeakHoldToggles = 0;
}
