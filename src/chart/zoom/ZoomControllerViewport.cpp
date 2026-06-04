/**
 * @file ZoomControllerViewport.cpp
 * @brief 图表缩放控制器 — 缩放范围管理与视口交互实现
 *
 * 本文件从 ZoomController.cpp 拆分而来，包含:
 * - 缩放状态栈管理 (pushZoomState / popZoomState)
 * - 以指定像素点为中心的缩放核心方法 (zoomAt)
 * - 滚轮缩放处理器 (handleWheel)
 * - 鼠标框选缩放/平移交互处理器
 *   (handleMousePress / handleMouseMove / handleMouseRelease)
 * - 双击重置处理器 (handleMouseDoubleClick)
 *
 * 拆分目的: 将视口范围管理与交互处理从核心事件分发和构造逻辑中分离，
 * 降低单文件行数，提升可维护性。
 *
 * @see ZoomController.cpp — 构造函数与事件过滤器主入口
 * @see ZoomControllerQuery.cpp — 状态查询与快捷缩放接口
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
// 缩放状态管理
// ============================================================

/** @brief 将当前axis范围压入zoom栈 */
void ZoomController::pushZoomState()
{
    ZoomState st;
    if (auto ax = xAxis()) {
        st.minX = ax->min(); st.maxX = ax->max();
    }
    if (auto ay = yAxis()) {
        st.minY = ay->min(); st.maxY = ay->max();
    }
    m_zoomStack.push(st);
}

/** @brief 从zoom栈弹出上一步范围并恢复 @return true=成功恢复 */
bool ZoomController::popZoomState()
{
    if (m_zoomStack.isEmpty()) return false;
    ZoomState st = m_zoomStack.pop();
    if (auto ax = xAxis()) ax->setRange(st.minX, st.maxX);
    if (auto ay = yAxis()) ay->setRange(st.minY, st.maxY);
    /* 防御: 若弹出状态范围为0(缩放到单点)，zoomLevel设为1.0 */
    const double rangeX = st.maxX - st.minX;
    m_zoomLevel = (rangeX > 0.0)
        ? (m_originalRange.maxX - m_originalRange.minX) / rangeX
        : 1.0;
    emit viewChanged();
    return true;
}

/** @brief 以指定像素点为中心缩放，保持centerPixelX对应的数据点不动，左右范围按factor缩放 @param centerPixelX 中心像素X坐标 @param factor 缩放因子(>1=放大) */
void ZoomController::zoomAt(int centerPixelX, double factor)
{
    auto ax = xAxis();
    if (!ax || !m_chart) return;

    // 保存缩放前状态
    pushZoomState();

    // 将像素中心映射到数据坐标
    double centerData = m_chart->mapToValue(
        QPointF(centerPixelX, 0)).x();

    double minVal = ax->min();
    double maxVal = ax->max();
    double range = maxVal - minVal;
    if (range <= 0) return;

    // 防止过度缩放
    double newRange = range / factor;
    if (newRange < kMinZoomLevel) {
        m_zoomStack.pop();
        return;
    }

    // 以centerData为中心缩放
    double ratio = (centerData - minVal) / range;
    double newMin = centerData - ratio * newRange;
    double newMax = newMin + newRange;

    ax->setRange(newMin, newMax);
    m_zoomLevel *= factor;
    ++m_totalZooms;
    ++m_totalZoomOperations;
    emit viewChanged();
}

// ============================================================
// 事件处理器
// ============================================================

/** @brief 处理滚轮缩放，angleDelta().y()>0放大，<0缩小，以鼠标位置为缩放中心 @param event 滚轮事件 */
void ZoomController::handleWheel(QWheelEvent* event)
{
    double factor = event->angleDelta().y() > 0 ? kZoomFactor
                                                 : 1.0 / kZoomFactor;
    zoomAt(event->position().x(), factor);
    event->accept();
}

/** @brief 处理鼠标按下，Ctrl+左键启动框选缩放，中键启动平移 @param event 鼠标事件 */
void ZoomController::handleMousePress(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton &&
        event->modifiers() & Qt::ControlModifier) {
        // 开始框选缩放
        m_rubberBandActive = true;
        m_rubberBandStart = event->pos();
        m_rubberBandEnd = event->pos();
    } else if (event->button() == Qt::MiddleButton) {
        // 开始平移
        m_panning = true;
        m_panStart = event->pos();
        if (auto ax = xAxis()) {
            m_panStartMinX = ax->min();
            m_panStartMaxX = ax->max();
        }
        if (auto ay = yAxis()) {
            m_panStartMinY = ay->min();
            m_panStartMaxY = ay->max();
        }
    }
}

/** @brief 处理鼠标移动，框选模式更新终点，平移模式根据偏移量调整axis范围 @param event 鼠标事件 */
void ZoomController::handleMouseMove(QMouseEvent* event)
{
    if (m_rubberBandActive) {
        m_rubberBandEnd = event->pos();
        emit viewChanged();  // 触发叠加层重绘以显示实时框选矩形
    } else if (m_panning && m_chart) {
        QPoint delta = event->pos() - m_panStart;
        auto ax = xAxis();
        auto ay = yAxis();

        if (ax) {
            double xRange = m_panStartMaxX - m_panStartMinX;
            if (xRange > 0 && m_chartView->width() > 0) {
                double dx = -delta.x() * xRange / m_chartView->width();
                ax->setRange(m_panStartMinX + dx, m_panStartMaxX + dx);
            }
        }
        if (ay) {
            double yRange = m_panStartMaxY - m_panStartMinY;
            if (yRange > 0 && m_chartView->height() > 0) {
                double dy = delta.y() * yRange / m_chartView->height();
                ay->setRange(m_panStartMinY + dy, m_panStartMaxY + dy);
            }
        }
        emit viewChanged();
    }
}

/** @brief 处理鼠标释放，完成框选缩放，框选区域<5像素时忽略 @param event 鼠标事件 */
void ZoomController::handleMouseRelease(QMouseEvent* event)
{
    if (m_rubberBandActive && event->button() == Qt::LeftButton) {
        m_rubberBandEnd = event->pos();
        m_rubberBandActive = false;

        QRectF rubberRect = QRectF(m_rubberBandStart, m_rubberBandEnd).normalized();
        // 框选区域太小则忽略
        if (rubberRect.width() < 5 || rubberRect.height() < 5) return;

        if (m_chart) {
            pushZoomState();

            // 将框选矩形映射到数据坐标
            QPointF topLeft = m_chart->mapToValue(rubberRect.topLeft());
            QPointF bottomRight = m_chart->mapToValue(rubberRect.bottomRight());

            if (auto ax = xAxis()) {
                double minV = qMin(topLeft.x(), bottomRight.x());
                double maxV = qMax(topLeft.x(), bottomRight.x());
                ax->setRange(minV, maxV);
            }
            if (auto ay = yAxis()) {
                double minV = qMin(bottomRight.y(), topLeft.y());
                double maxV = qMax(bottomRight.y(), topLeft.y());
                ay->setRange(minV, maxV);
            }

            // 更新缩放倍率
            if (auto ax = xAxis()) {
                double origRange = m_originalRange.maxX - m_originalRange.minX;
                double curRange = ax->max() - ax->min();
                if (curRange > 0) m_zoomLevel = origRange / curRange;
            }

            ++m_totalZooms;
            ++m_totalZoomOperations;
            emit viewChanged();
        }
    } else if (m_panning && event->button() == Qt::MiddleButton) {
        m_panning = false;
        ++m_totalPans;
        ++m_totalZoomOperations;
    }
}

/** @brief 处理鼠标双击，重置缩放到原始范围 @param event 鼠标事件 */
void ZoomController::handleMouseDoubleClick(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        resetZoom();
    }
}
