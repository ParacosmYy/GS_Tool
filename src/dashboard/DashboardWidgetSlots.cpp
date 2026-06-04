/**
 * @file DashboardWidgetSlots.cpp
 * @brief 仪表盘主容器 — 事件处理器与统计接口实现
 *
 * 从DashboardWidget.cpp拆分，负责:
 *   1. paintEvent: 重绘事件处理，累计重绘计数
 *   2. 统计Getter接口: 布局变更/添加/移除/保存/加载/更新/值变更/范围变更/重绘
 *   3. resetDashboardWidgetStatistics: 重置所有统计计数器
 */

#include "dashboard/DashboardWidget.h"

#include <QPaintEvent>

/** @brief 重绘事件，累计重绘计数 @param event 绘制事件 */
void DashboardWidget::paintEvent(QPaintEvent *event)
{
    ++m_totalRepaints;
    QWidget::paintEvent(event);
}

// ─── 统计接口 ───────────────────────────────────────────────────────

/** @brief 获取累计布局变更次数 @return 变更次数 */
quint64 DashboardWidget::totalLayoutChanges() const
{
    return m_totalLayoutChanges;
}

/** @brief 获取累计添加组件次数 @return 添加次数 */
quint64 DashboardWidget::totalWidgetsAdded() const
{
    return m_totalWidgetsAdded;
}

/** @brief 获取累计移除组件次数 @return 移除次数 */
quint64 DashboardWidget::totalWidgetsRemoved() const
{
    return m_totalWidgetsRemoved;
}

/** @brief 获取累计完整序列化保存次数 @return 保存次数 */
quint64 DashboardWidget::totalFullSaves() const
{
    return m_totalFullSaves;
}

/** @brief 获取累计完整序列化加载次数 @return 加载次数 */
quint64 DashboardWidget::totalFullLoads() const
{
    return m_totalFullLoads;
}

/** @brief 重置所有仪表盘容器统计计数器 */
void DashboardWidget::resetDashboardWidgetStatistics()
{
    m_totalLayoutChanges = 0;
    m_totalWidgetsAdded = 0;
    m_totalWidgetsRemoved = 0;
    m_totalFullSaves = 0;
    m_totalFullLoads = 0;
    m_totalUpdates = 0;
    m_totalValueChanged = 0;
    m_totalRangeChanges = 0;
    m_totalRepaints = 0;
}

/** @brief 获取累计组件更新次数 @return 更新总数 */
quint64 DashboardWidget::totalUpdates() const { return m_totalUpdates; }

/** @brief 获取累计值变更通知次数 @return 变更总数 */
quint64 DashboardWidget::totalValueChanged() const { return m_totalValueChanged; }

/** @brief 获取累计范围变更次数 @return 范围变更总数 */
quint64 DashboardWidget::totalRangeChanges() const { return m_totalRangeChanges; }

/** @brief 获取累计重绘次数 @return 重绘总数 */
quint64 DashboardWidget::totalRepaints() const { return m_totalRepaints; }
