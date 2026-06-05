/**
 * @file GpsTrackWidget.cpp
 * @brief GPS 轨迹可视化控件核心实现
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * Mercator 投影渲染、网格标注、轨迹折线、当前位置箭头、航路点标记。
 */

#include "utils/gps/GpsTrackWidget.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

/** @brief 构造轨迹控件 @param parent 父控件 */
GpsTrackWidget::GpsTrackWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("GpsTrackWidget"));
    setMinimumSize(200, 150);
    setMouseTracking(true);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/** @brief 设置轨迹数据 @param points 轨迹点列表 */
void GpsTrackWidget::setTrackPoints(const QVector<GpsTrackPoint> &points)
{
    m_trackPoints = points;
    computeBounds();
    fitToTrack();
    update();
}

/** @brief 追加单个轨迹点 @param point 新轨迹点 */
void GpsTrackWidget::appendTrackPoint(const GpsTrackPoint &point)
{
    m_trackPoints.append(point);
    // Update bounds incrementally
    if (m_trackPoints.size() == 1) {
        m_minLat = m_maxLat = point.pos.latitude;
        m_minLon = m_maxLon = point.pos.longitude;
        m_boundsValid = true;
    } else {
        if (point.pos.latitude < m_minLat) m_minLat = point.pos.latitude;
        if (point.pos.latitude > m_maxLat) m_maxLat = point.pos.latitude;
        if (point.pos.longitude < m_minLon) m_minLon = point.pos.longitude;
        if (point.pos.longitude > m_maxLon) m_maxLon = point.pos.longitude;
    }
    update();
}

/** @brief 设置当前位置标记 @param position 当前 GPS 位置 */
void GpsTrackWidget::setCurrentPosition(const GpsPosition &position)
{
    m_currentPos = position;
    update();
}

/** @brief 添加航路点标记 @param lat 纬度 @param lon 经度 @param label 标签 */
void GpsTrackWidget::addWaypoint(double lat, double lon, const QString &label)
{
    m_waypoints.append({lat, lon, label});
    ++m_stats.waypointMarkers;
    update();
}

/** @brief 清除所有轨迹和标记 */
void GpsTrackWidget::clearTrack()
{
    m_trackPoints.clear();
    m_waypoints.clear();
    m_currentPos = GpsPosition();
    m_boundsValid = false;
    m_minLat = 90.0;  m_maxLat = -90.0;
    m_minLon = 180.0; m_maxLon = -180.0;
    update();
}

/** @brief 自动适配视口到轨迹边界 */
void GpsTrackWidget::fitToTrack()
{
    if (!m_boundsValid) {
        return;
    }
    ++m_stats.autoFits;

    const double latRange = m_maxLat - m_minLat;
    const double lonRange = m_maxLon - m_minLon;

    // Add 10% margin
    const double margin = 0.1;
    const double effLatRange = qMax(latRange * (1.0 + margin), 0.001);
    const double effLonRange = qMax(lonRange * (1.0 + margin), 0.001);

    m_centerLat = (m_minLat + m_maxLat) / 2.0;
    m_centerLon = (m_minLon + m_maxLon) / 2.0;

    const double scaleX = width()  / effLonRange;
    const double scaleY = height() / effLatRange;
    m_scale = qMin(scaleX, scaleY);
    update();
}

/** @brief 控件推荐大小 @return 推荐尺寸 */
QSize GpsTrackWidget::sizeHint() const
{
    return QSize(600, 400);
}

/** @brief 控件最小推荐大小 @return 最小尺寸 */
QSize GpsTrackWidget::minimumSizeHint() const
{
    return QSize(200, 150);
}

// ---------------------------------------------------------------------------
// Coordinate transform
// ---------------------------------------------------------------------------

/** @brief GPS 经纬度 → 像素坐标 @param lat 纬度 @param lon 经度 @return 像素坐标 */
QPointF GpsTrackWidget::geoToPixel(double lat, double lon) const
{
    const double halfW = width()  / 2.0;
    const double halfH = height() / 2.0;
    // Simple Mercator: Y inverted (lat increases upward)
    const double px = halfW + (lon - m_centerLon) * m_scale;
    const double py = halfH - (lat - m_centerLat) * m_scale;
    return QPointF(px, py);
}

/** @brief 像素坐标 → GPS 经纬度 @param px 像素坐标 @return QPointF(经度, 纬度) */
QPointF GpsTrackWidget::pixelToGeo(const QPointF &px) const
{
    const double halfW = width()  / 2.0;
    const double halfH = height() / 2.0;
    const double lon = m_centerLon + (px.x() - halfW) / m_scale;
    const double lat = m_centerLat - (px.y() - halfH) / m_scale;
    return QPointF(lon, lat);
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

/** @brief 主绘制事件 @param event 绘制事件 */
void GpsTrackWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    ++m_stats.redraws;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Background
    painter.fillRect(rect(), QColor(30, 30, 40));

    drawGrid(painter);
    drawTrack(painter);
    drawWaypoints(painter);
    drawCurrentPosition(painter);
}

/** @brief 绘制网格和经纬度标注 */
void GpsTrackWidget::drawGrid(QPainter &painter)
{
    if (m_scale <= 0) {
        return;
    }
    ++m_stats.gridRenders;

    const QPen gridPen(QColor(60, 60, 80), 1, Qt::DotLine);
    const QPen labelPen(QColor(140, 140, 160));
    const QFont labelFont(QStringLiteral("Consolas"), 8);

    painter.setPen(gridPen);
    painter.setFont(labelFont);

    // Calculate visible geo range
    const QPointF topLeft     = pixelToGeo(QPointF(0, 0));
    const QPointF bottomRight = pixelToGeo(QPointF(width(), height()));
    const double lonRange = topLeft.x() - bottomRight.x();
    const double latRange = bottomRight.y() - topLeft.y();

    const double step = niceGridStep(qMax(qAbs(lonRange), qAbs(latRange)));

    // Draw longitude lines
    const double lonStart = qFloor(topLeft.x() / step) * step;
    for (double lon = lonStart; lon <= bottomRight.x(); lon += step) {
        const QPointF p1 = geoToPixel(90, lon);
        const QPointF p2 = geoToPixel(-90, lon);
        painter.drawLine(p1, p2);
        painter.setPen(labelPen);
        painter.drawText(static_cast<int>(p1.x()) + 2, 12,
                         QString::number(lon, 'f', step < 0.01 ? 4 : 2) +
                         QStringLiteral("\u00B0"));
        painter.setPen(gridPen);
    }

    // Draw latitude lines
    const double latStart = qFloor(bottomRight.y() / step) * step;
    for (double lat = latStart; lat <= topLeft.y(); lat += step) {
        const QPointF p1 = geoToPixel(lat, -180);
        const QPointF p2 = geoToPixel(lat, 180);
        painter.drawLine(p1, p2);
        painter.setPen(labelPen);
        painter.drawText(2, static_cast<int>(geoToPixel(lat, 0).y()) - 2,
                         QString::number(lat, 'f', step < 0.01 ? 4 : 2) +
                         QStringLiteral("\u00B0"));
        painter.setPen(gridPen);
    }
}

/** @brief 绘制轨迹折线 */
void GpsTrackWidget::drawTrack(QPainter &painter)
{
    if (m_trackPoints.size() < 2) {
        return;
    }
    m_stats.trackPointsDrawn += static_cast<quint64>(m_trackPoints.size());

    QPen trackPen(QColor(0, 200, 100), 2);
    trackPen.setCapStyle(Qt::RoundCap);
    trackPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(trackPen);

    QPainterPath path;
    const QPointF first = geoToPixel(m_trackPoints[0].pos.latitude,
                                      m_trackPoints[0].pos.longitude);
    path.moveTo(first);
    for (int i = 1; i < m_trackPoints.size(); ++i) {
        const QPointF pt = geoToPixel(m_trackPoints[i].pos.latitude,
                                       m_trackPoints[i].pos.longitude);
        path.lineTo(pt);
    }
    painter.drawPath(path);
}

/** @brief 绘制当前位置标记(含方向箭头) */
void GpsTrackWidget::drawCurrentPosition(QPainter &painter)
{
    if (!m_currentPos.valid) {
        return;
    }
    const QPointF center = geoToPixel(m_currentPos.latitude,
                                       m_currentPos.longitude);

    // Outer circle
    painter.setPen(QPen(QColor(255, 80, 80), 2));
    painter.setBrush(QColor(255, 80, 80, 180));
    painter.drawEllipse(center, 6, 6);

    // Direction arrow
    if (m_currentPos.course >= 0) {
        const double rad = qDegreesToRadians(m_currentPos.course);
        const double arrowLen = 14.0;
        const QPointF tip(center.x() + arrowLen * qSin(rad),
                          center.y() - arrowLen * qCos(rad));
        painter.setPen(QPen(QColor(255, 200, 80), 2));
        painter.drawLine(center, tip);
    }
}

/** @brief 绘制航路点标记 */
void GpsTrackWidget::drawWaypoints(QPainter &painter)
{
    if (m_waypoints.isEmpty()) {
        return;
    }
    painter.setPen(QPen(QColor(80, 160, 255), 2));
    painter.setBrush(QColor(80, 160, 255, 120));
    const QFont wpFont(QStringLiteral("Segoe UI"), 8);
    painter.setFont(wpFont);

    for (const auto &wp : m_waypoints) {
        const QPointF pt = geoToPixel(wp.lat, wp.lon);
        // Diamond marker
        QPainterPath diamond;
        diamond.moveTo(pt.x(), pt.y() - 6);
        diamond.lineTo(pt.x() + 5, pt.y());
        diamond.lineTo(pt.x(), pt.y() + 6);
        diamond.lineTo(pt.x() - 5, pt.y());
        diamond.closeSubpath();
        painter.drawPath(diamond);
        if (!wp.label.isEmpty()) {
            painter.setPen(QColor(200, 220, 255));
            painter.drawText(static_cast<int>(pt.x()) + 6,
                             static_cast<int>(pt.y()) - 4, wp.label);
            painter.setPen(QPen(QColor(80, 160, 255), 2));
        }
    }
}

// ---------------------------------------------------------------------------
// Bounds
// ---------------------------------------------------------------------------

/** @brief 计算轨迹经纬度边界 */
void GpsTrackWidget::computeBounds()
{
    m_boundsValid = false;
    if (m_trackPoints.isEmpty()) {
        return;
    }
    m_minLat = 90.0;  m_maxLat = -90.0;
    m_minLon = 180.0; m_maxLon = -180.0;
    for (const auto &tp : m_trackPoints) {
        if (tp.pos.latitude  < m_minLat) m_minLat = tp.pos.latitude;
        if (tp.pos.latitude  > m_maxLat) m_maxLat = tp.pos.latitude;
        if (tp.pos.longitude < m_minLon) m_minLon = tp.pos.longitude;
        if (tp.pos.longitude > m_maxLon) m_maxLon = tp.pos.longitude;
    }
    m_boundsValid = true;
}

/** @brief 计算良好的网格间距 @param range 经纬度范围 @return 建议的网格步长(度) */
double GpsTrackWidget::niceGridStep(double range)
{
    if (range <= 0) {
        return 1.0;
    }
    const double rough = range / 5.0;
    const double mag   = qPow(10.0, qFloor(qLn(rough) / qLn(10.0)));
    const double norm  = rough / mag;
    double nice;
    if (norm < 1.5) {
        nice = 1.0;
    } else if (norm < 3.0) {
        nice = 2.0;
    } else if (norm < 7.0) {
        nice = 5.0;
    } else {
        nice = 10.0;
    }
    return nice * mag;
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

/** @brief 大小改变事件 @param event 调整大小事件 */
void GpsTrackWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_boundsValid) {
        fitToTrack();
    }
}

/** @brief 鼠标按下 @param event 鼠标事件 */
void GpsTrackWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->position();
        setCursor(Qt::ClosedHandCursor);

        // Emit positionClicked
        const QPointF geo = pixelToGeo(event->position());
        GpsPosition clickPos;
        clickPos.latitude  = geo.y();
        clickPos.longitude = geo.x();
        emit positionClicked(clickPos);
    }
}

/** @brief 鼠标移动(拖拽平移) @param event 鼠标事件 */
void GpsTrackWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        const QPointF delta = event->position() - m_lastMousePos;
        m_centerLon -= delta.x() / m_scale;
        m_centerLat += delta.y() / m_scale;
        m_lastMousePos = event->position();
        ++m_stats.panEvents;
        update();
    }
}

/** @brief 鼠标释放 @param event 鼠标事件 */
void GpsTrackWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
}

/** @brief 滚轮缩放 @param event 滚轮事件 */
void GpsTrackWidget::wheelEvent(QWheelEvent *event)
{
    const double factor = (event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    m_scale *= factor;
    m_scale = qBound(1.0, m_scale, 1e8);
    ++m_stats.zoomEvents;
    update();
}
