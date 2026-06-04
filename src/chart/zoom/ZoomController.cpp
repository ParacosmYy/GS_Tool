/**
 * @file ZoomController.cpp
 * @brief 图表缩放/平移状态控制器实现
 *
 * 通过eventFilter拦截QChartView的鼠标/滚轮事件，
 * 实现以光标为中心的滚轮缩放、Ctrl+左键框选缩放、
 * 中键拖拽平移和双击恢复原始范围。
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
// 构造
// ============================================================

/** @brief 构造缩放控制器，缓存QChart指针和初始axis范围用于后续resetZoom恢复 @param chartView 关联的图表视图 @param parent 父对象 */
ZoomController::ZoomController(QChartView* chartView, QObject* parent)
    : QObject(parent)
    , m_chartView(chartView)
    , m_chart(chartView ? chartView->chart() : nullptr)
{
    if (m_chart) {
        // 记录初始axis范围
        if (auto ax = xAxis()) {
            m_originalRange.minX = ax->min();
            m_originalRange.maxX = ax->max();
        } else {
            m_originalRange.minX = 0;
            m_originalRange.maxX = 100;
        }
        if (auto ay = yAxis()) {
            m_originalRange.minY = ay->min();
            m_originalRange.maxY = ay->max();
        } else {
            m_originalRange.minY = 0;
            m_originalRange.maxY = 100;
        }
    }
}

// ============================================================
// 公开接口 — 状态查询/快捷缩放/resetZoom 已拆分至 ZoomControllerQuery.cpp
// ============================================================

// ============================================================
// 事件过滤
// ============================================================

/** @brief 事件过滤器主入口，分发WheelEvent/Press/Move/Release/DblClick事件 @param watched 被观察的对象 @param event 事件对象 @return true=事件已处理不再传递 */
bool ZoomController::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (!m_chartView || !m_chart) return false;

    switch (event->type()) {
    case QEvent::Wheel:
        handleWheel(static_cast<QWheelEvent*>(event));
        return true;    // 滚轮缩放始终消费
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        // 只拦截Ctrl+左键(框选)和中键(平移)
        if (me->button() == Qt::LeftButton &&
            me->modifiers() & Qt::ControlModifier) {
            handleMousePress(me);
            return true;
        }
        if (me->button() == Qt::MiddleButton) {
            handleMousePress(me);
            return true;
        }
        return false;   // 其他鼠标按下事件放行
    }
    case QEvent::MouseMove:
        if (m_rubberBandActive || m_panning) {
            handleMouseMove(static_cast<QMouseEvent*>(event));
            return true;
        }
        return false;   // 非拖拽状态放行
    case QEvent::MouseButtonRelease:
        if (m_rubberBandActive || m_panning) {
            handleMouseRelease(static_cast<QMouseEvent*>(event));
            return true;
        }
        return false;   // 非拖拽状态放行
    case QEvent::MouseButtonDblClick: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            handleMouseDoubleClick(me);
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

// ============================================================
// 缩放状态管理 / 事件处理器 — 已拆分至 ZoomControllerViewport.cpp
// 辅助方法 / 统计计数器 — 已拆分至 ZoomControllerStats.cpp
// 状态查询 / 快捷缩放 — 已拆分至 ZoomControllerQuery.cpp
// ============================================================
