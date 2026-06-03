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

/**
 * @brief 构造缩放控制器
 * @param chartView 关联的图表视图
 * @param parent 父对象
 *
 * 缓存QChart指针和初始axis范围，用于后续resetZoom恢复。
 */
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
// 公开接口
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

/** @brief 放大 — 以图表中心为缩放中心 */
void ZoomController::zoomIn()
{
    if (!m_chartView) return;
    int center = m_chartView->width() / 2;
    zoomAt(center, kZoomFactor);
}

/** @brief 缩小 — 以图表中心为缩放中心 */
void ZoomController::zoomOut()
{
    if (!m_chartView) return;
    int center = m_chartView->width() / 2;
    zoomAt(center, 1.0 / kZoomFactor);
}

// ============================================================
// 事件过滤
// ============================================================

/**
 * @brief 事件过滤器主入口
 * @return true=事件已处理，不再传递
 *
 * 事件分发:
 * - WheelEvent → handleWheel (滚轮缩放)
 * - MouseButtonPress → handleMousePress (框选/平移)
 * - MouseMove → handleMouseMove (框选绘制)
 * - MouseButtonRelease → handleMouseRelease (完成框选)
 * - MouseButtonDblClick → handleMouseDoubleClick (重置)
 */
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

/**
 * @brief 以指定像素点为中心缩放
 * @param centerPixelX 中心像素X坐标
 * @param factor 缩放因子(>1=放大)
 *
 * 保持centerPixelX对应的数据点不动，左右范围按factor缩放。
 */
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

/**
 * @brief 处理滚轮缩放
 *
 * angleDelta().y() > 0 → 放大，< 0 → 缩小。
 * 以鼠标位置为缩放中心。
 */
void ZoomController::handleWheel(QWheelEvent* event)
{
    double factor = event->angleDelta().y() > 0 ? kZoomFactor
                                                 : 1.0 / kZoomFactor;
    zoomAt(event->position().x(), factor);
    event->accept();
}

/**
 * @brief 处理鼠标按下
 *
 * - Ctrl+左键 → 框选缩放(记录起点)
 * - 中键 → 平移(记录起点和初始axis范围)
 */
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

/**
 * @brief 处理鼠标移动
 *
 * 框选模式: 更新终点(绘制由ChartWidget负责)
 * 平移模式: 根据偏移量调整axis范围
 */
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

/**
 * @brief 处理鼠标释放 — 完成框选缩放
 *
 * 如果框选区域太小(<5像素)，忽略此次操作。
 * 否则将框选区域映射到数据坐标并设置为新的axis范围。
 */
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

/**
 * @brief 处理鼠标双击 — 重置缩放到原始范围
 */
void ZoomController::handleMouseDoubleClick(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        resetZoom();
    }
}

// ============================================================
// 辅助
// ============================================================

/** @brief 获取X轴对象(第一个QValueAxis) */
QValueAxis* ZoomController::xAxis() const
{
    if (!m_chart) return nullptr;
    for (auto* axis : m_chart->axes(Qt::Horizontal)) {
        if (auto* va = qobject_cast<QValueAxis*>(axis)) return va;
    }
    return nullptr;
}

/** @brief 获取Y轴对象(第一个QValueAxis) */
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

/** @brief 返回缩放操作总次数 */
quint64 ZoomController::totalZooms() const
{
    return m_totalZooms;
}

/** @brief 返回平移操作总次数 */
quint64 ZoomController::totalPans() const
{
    return m_totalPans;
}

/** @brief 返回缩放重置总次数 */
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
}
