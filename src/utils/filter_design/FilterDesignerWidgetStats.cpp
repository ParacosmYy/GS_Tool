/**
 * @file FilterDesignerWidgetStats.cpp
 * @brief 滤波器设计控件统计接口实现 — 计数器查询与重置
 *
 * 从 FilterDesignerWidget.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "utils/filter_design/FilterDesignerWidget.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计重绘次数 */
quint64 FilterDesignerWidget::totalRepaints() const
{
    return m_totalRepaints;
}

/** @brief 获取累计参数变更次数 */
quint64 FilterDesignerWidget::totalParamChanges() const
{
    return m_totalParamChanges;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void FilterDesignerWidget::resetStatistics()
{
    m_totalRepaints     = 0;
    m_totalParamChanges = 0;
}
