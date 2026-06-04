/**
 * @file ZoomControllerStats.cpp
 * @brief 图表缩放控制器 — 辅助方法与统计计数器实现
 *
 * 本文件从 ZoomController.cpp 拆分而来，包含:
 * - X/Y轴对象获取辅助方法 (xAxis / yAxis)
 * - 缩放统计计数器的查询与重置接口
 *   (totalZooms / totalPans / totalResets / resetZoomStatistics)
 *
 * 拆分目的: 将统计/辅助逻辑与核心缩放/事件处理逻辑分离，
 * 降低单文件行数，提升可维护性。
 */

#include "chart/zoom/ZoomController.h"

#include <QChartView>
#include <QChart>
#include <QValueAxis>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QtMath>

// ============================================================
// 辅助
// ============================================================

/** @brief 获取X轴对象(第一个QValueAxis) @return X轴指针，不存在返回nullptr */
QValueAxis* ZoomController::xAxis() const
{
    if (!m_chart) return nullptr;
    for (auto* axis : m_chart->axes(Qt::Horizontal)) {
        if (auto* va = qobject_cast<QValueAxis*>(axis)) return va;
    }
    return nullptr;
}

/** @brief 获取Y轴对象(第一个QValueAxis) @return Y轴指针，不存在返回nullptr */
QValueAxis* ZoomController::yAxis() const
{
    if (!m_chart) return nullptr;
    for (auto* axis : m_chart->axes(Qt::Vertical)) {
        if (auto* va = qobject_cast<QValueAxis*>(axis)) return va;
    }
    return nullptr;
}

// ============================================================
// 统计计数器接口
// ============================================================

/** @brief 返回缩放操作总次数 @return 缩放次数 */
quint64 ZoomController::totalZooms() const
{
    return m_totalZooms;
}

/** @brief 返回平移操作总次数 @return 平移次数 */
quint64 ZoomController::totalPans() const
{
    return m_totalPans;
}

/** @brief 返回缩放重置总次数 @return 重置次数 */
quint64 ZoomController::totalResets() const
{
    return m_totalResets;
}

/** @brief 重置所有缩放统计计数器为初始值 */
void ZoomController::resetZoomStatistics()
{
    m_totalZooms = 0;
    m_totalPans = 0;
    m_totalResets = 0;
    m_totalZoomOperations = 0;
    m_totalUndos = 0;
}
