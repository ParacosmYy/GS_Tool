/**
 * @file GpsTrackWidget.h
 * @brief GPS 轨迹可视化控件
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 自绘 QWidget，使用 Mercator 投影渲染 GPS 轨迹折线、
 * 当前位置标记、航路点、网格及经纬度标注。
 * 支持鼠标滚轮缩放和拖拽平移。
 */

#ifndef GPSTRACKWIDGET_H
#define GPSTRACKWIDGET_H

#include <QPointF>
#include <QVector>
#include <QWidget>

#include "utils/gps/GpsTypes.h"

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

/**
 * @class GpsTrackWidget
 * @brief GPS 轨迹可视化控件 -- Mercator 投影 + 缩放/平移
 */
class GpsTrackWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造轨迹控件 @param parent 父控件 */
    explicit GpsTrackWidget(QWidget *parent = nullptr);

    /** @brief 设置轨迹数据 @param points 轨迹点列表 */
    void setTrackPoints(const QVector<GpsTrackPoint> &points);

    /** @brief 追加单个轨迹点 @param point 新轨迹点 */
    void appendTrackPoint(const GpsTrackPoint &point);

    /** @brief 设置当前位置标记 @param position 当前 GPS 位置 */
    void setCurrentPosition(const GpsPosition &position);

    /** @brief 添加航路点标记 @param lat 纬度 @param lon 经度 @param label 标签文本 */
    void addWaypoint(double lat, double lon, const QString &label = QString());

    /** @brief 清除所有轨迹和标记 */
    void clearTrack();

    /** @brief 自动适配视口到轨迹边界 */
    void fitToTrack();

    // ---- 统计接口 ----
    /** @brief 获取统计快照 @return 统计结构体 */
    GpsTrackWidgetStats trackStats() const { return m_stats; }
    /** @brief 重置轨迹控件统计计数器 */
    void resetTrackStats();

    /** @brief 控件推荐大小 @return 推荐尺寸 */
    QSize sizeHint() const override;
    /** @brief 控件最小推荐大小 @return 最小尺寸 */
    QSize minimumSizeHint() const override;

signals:
    /** @brief 用户点击地图位置时发射 @param position 点击位置对应的 GPS 坐标 */
    void positionClicked(const GpsPosition &position);

protected:
    /** @brief 绘制事件 */
    void paintEvent(QPaintEvent *event) override;
    /** @brief 大小改变事件 */
    void resizeEvent(QResizeEvent *event) override;
    /** @brief 鼠标按下事件 */
    void mousePressEvent(QMouseEvent *event) override;
    /** @brief 鼠标移动事件(拖拽平移) */
    void mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 鼠标释放事件 */
    void mouseReleaseEvent(QMouseEvent *event) override;
    /** @brief 滚轮事件(缩放) */
    void wheelEvent(QWheelEvent *event) override;

private:
    /** @brief GPS 经纬度转控件坐标 @param lat 纬度 @param lon 经度 @return 像素坐标 */
    QPointF geoToPixel(double lat, double lon) const;

    /** @brief 控件坐标转 GPS 经纬度 @param px 像素坐标 @return QPointF(经度, 纬度) */
    QPointF pixelToGeo(const QPointF &px) const;

    /** @brief 绘制网格和经纬度标注 */
    void drawGrid(QPainter &painter);

    /** @brief 绘制轨迹折线 */
    void drawTrack(QPainter &painter);

    /** @brief 绘制当前位置标记(含方向箭头) */
    void drawCurrentPosition(QPainter &painter);

    /** @brief 绘制航路点标记 */
    void drawWaypoints(QPainter &painter);

    /** @brief 计算轨迹的经纬度边界 */
    void computeBounds();

    /** @brief 良好网格间距(度) @param range 经纬度范围 */
    static double niceGridStep(double range);

    // ---- 数据 ----
    QVector<GpsTrackPoint> m_trackPoints;   ///< 轨迹点
    GpsPosition            m_currentPos;    ///< 当前位置
    struct Waypoint {
        double lat;
        double lon;
        QString label;
    };
    QVector<Waypoint> m_waypoints;          ///< 航路点

    // ---- 视口参数 ----
    double m_centerLat = 0.0;               ///< 视口中心纬度
    double m_centerLon = 0.0;               ///< 视口中心经度
    double m_scale     = 1.0;               ///< 像素/度
    double m_minLat = 90.0, m_maxLat = -90.0; ///< 轨迹纬度边界
    double m_minLon = 180.0, m_maxLon = -180.0; ///< 轨迹经度边界
    bool   m_boundsValid = false;           ///< 边界是否已计算

    // ---- 交互状态 ----
    bool     m_dragging = false;            ///< 是否正在拖拽
    QPointF  m_lastMousePos;                ///< 上次鼠标位置

    // ---- 统计 ----
    GpsTrackWidgetStats m_stats;            ///< 统计
};

#endif // GPSTRACKWIDGET_H
