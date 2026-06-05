/**
 * @file EyeDiagramWidgetStats.cpp
 * @brief 眼图控件统计接口实现 — 计数器查询与重置
 *
 * 从 EyeDiagramWidget.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "chart/eye/EyeDiagramWidget.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计重绘次数 */
quint64 EyeDiagramWidget::totalRepaints() const
{
    return m_totalRepaints;
}

/** @brief 获取累计掩模显隐切换次数 */
quint64 EyeDiagramWidget::totalMaskToggles() const
{
    return m_totalMaskToggles;
}

/** @brief 获取累计缩放事件次数 */
quint64 EyeDiagramWidget::totalZoomEvents() const
{
    return m_totalZoomEvents;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void EyeDiagramWidget::resetStatistics()
{
    m_totalRepaints    = 0;
    m_totalMaskToggles = 0;
    m_totalZoomEvents  = 0;
}
