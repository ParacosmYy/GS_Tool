/**
 * @file ZoomControllerQuery.cpp
 * @brief 图表缩放控制器 — 状态查询与快捷缩放接口实现
 *
 * 本文件从 ZoomController.cpp 拆分而来，包含:
 * - 缩放倍率/框选状态/框选矩形的查询接口
 * - zoomIn/zoomOut 快捷缩放方法
 *
 * 拆分目的: 将只读查询与快捷操作从核心事件处理逻辑中分离，
 * 降低单文件行数，提升可维护性。
 *
 * @see ZoomController.cpp — 核心缩放/事件处理逻辑
 * @see ZoomControllerStats.cpp — 辅助方法与统计计数器
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
// 状态查询
// ============================================================

/** @brief 获取当前缩放倍率 @return 缩放倍率(1.0=原始) */
double ZoomController::zoomLevel() const { return m_zoomLevel; }

/** @brief 查询是否处于框选缩放模式 @return true=正在框选 */
bool ZoomController::isRubberBandActive() const { return m_rubberBandActive; }

/** @brief 获取当前框选矩形(像素坐标) @return 框选区域，非框选时返回空矩形 */
QRectF ZoomController::rubberBandRect() const
{
    if (!m_rubberBandActive) return QRectF();
    return QRectF(m_rubberBandStart, m_rubberBandEnd).normalized();
}

// ============================================================
// 快捷缩放
// ============================================================

/** @brief 重置缩放至原始数据范围，清空zoom栈 */
void ZoomController::resetZoom()
{
    if (!m_chart) return;

    if (auto ax = xAxis()) {
        ax->setRange(m_originalRange.minX, m_originalRange.maxX);
    }
    if (auto ay = yAxis()) {
        ay->setRange(m_originalRange.minY, m_originalRange.maxY);
    }

    m_zoomStack.clear();
    m_zoomLevel = 1.0;
    ++m_totalResets;
    ++m_totalZoomOperations;
    emit viewChanged();
    emit zoomReset();
}

/** @brief 放大，以图表中心为缩放中心 */
void ZoomController::zoomIn()
{
    if (!m_chartView) return;
    int center = m_chartView->width() / 2;
    zoomAt(center, kZoomFactor);
}

/** @brief 缩小，以图表中心为缩放中心 */
void ZoomController::zoomOut()
{
    if (!m_chartView) return;
    int center = m_chartView->width() / 2;
    zoomAt(center, 1.0 / kZoomFactor);
}
