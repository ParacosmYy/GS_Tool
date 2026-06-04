/**
 * @file ZoomControllerViewport.cpp
 * @brief 图表缩放控制器 — 缩放状态管理与核心缩放方法
 *
 * 本文件从 ZoomController.cpp 拆分而来，包含:
 * - 缩放状态栈管理 (pushZoomState / popZoomState)
 * - 以指定像素点为中心的缩放核心方法 (zoomAt)
 *
 * 鼠标/滚轮事件处理器见 ZoomControllerHandlers.cpp。
 *
 * @see ZoomController.cpp — 构造函数与事件过滤器主入口
 * @see ZoomControllerQuery.cpp — 状态查询与快捷缩放接口
 * @see ZoomControllerStats.cpp — 辅助方法与统计计数器
 */

#include "chart/zoom/ZoomController.h"

#include <QChartView>
#include <QChart>
#include <QValueAxis>
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
    ++m_totalUndos;
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

// 事件处理器见 ZoomControllerHandlers.cpp
