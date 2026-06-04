/** @file ZoomController.h @brief 图表缩放/平移状态控制器。管理QChart缩放/平移/滚轮缩放(以光标为中心)/中键拖拽/框选缩放(Ctrl+左键)/双击恢复/undo栈 */
#ifndef CHART_ZOOMCONTROLLER_H
#define CHART_ZOOMCONTROLLER_H

#include <QObject>
#include <QRectF>
#include <QStack>

class QChartView;
class QChart;
class QValueAxis;
class QMouseEvent;
class QWheelEvent;

/**
 * @brief 缩放/平移控制器
 * 安装为QChartView的eventFilter后自动处理鼠标滚轮/拖拽事件。
 * 只操作QChart的axis范围，不触碰series数据。缩放/平移后发射信号通知CursorOverlay刷新。
 */
class ZoomController : public QObject {
    Q_OBJECT

public:
    explicit ZoomController(QChartView* chartView, QObject* parent = nullptr); ///< 构造
    void resetZoom();              ///< 重置缩放至原始数据范围
    double zoomLevel() const;      ///< 当前缩放倍率(1.0=原始)
    bool isRubberBandActive() const; ///< 是否框选模式
    QRectF rubberBandRect() const; ///< 当前框选矩形(像素坐标)
    // ---- 统计计数器接口 ----
    quint64 totalZooms() const;                 ///< 缩放操作总次数(含滚轮/框选/按钮)
    quint64 totalPans() const;                  ///< 平移操作总次数
    quint64 totalResets() const;                ///< 缩放重置总次数
    quint64 totalZoomOperations() const { return m_totalZoomOperations; } ///< 所有缩放操作(含缩放/平移/重置/undo)
    void resetZoomStatistics();                 ///< 重置所有缩放统计计数器

signals:
    void viewChanged();  ///< 缩放/平移变化，接收方应刷新叠加层
    void zoomReset();    ///< 缩放被重置到原始范围

public slots:
    void zoomIn();       ///< 放大一步(倍率x1.5)
    void zoomOut();      ///< 缩小一步(倍率/1.5)

protected:
    bool eventFilter(QObject* watched, QEvent* event) override; ///< 事件过滤器

private:
    void pushZoomState();  ///< 保存当前axis范围到zoom栈
    bool popZoomState();   ///< 从zoom栈恢复上一步
    /** @brief 以指定像素点为中心缩放 @param centerPixelX 中心X @param factor 缩放因子 */
    void zoomAt(int centerPixelX, double factor);
    void handleWheel(QWheelEvent* event);           ///< 滚轮缩放
    void handleMousePress(QMouseEvent* event);      ///< 鼠标按下(开始框选或平移)
    void handleMouseMove(QMouseEvent* event);       ///< 鼠标移动(框选绘制)
    void handleMouseRelease(QMouseEvent* event);    ///< 鼠标释放(完成框选缩放)
    void handleMouseDoubleClick(QMouseEvent* event); ///< 双击(重置缩放)
    QValueAxis* xAxis() const; ///< X轴对象
    QValueAxis* yAxis() const; ///< Y轴对象
    // ---- 成员 ----
    QChartView* m_chartView;           ///< 关联的图表视图
    QChart* m_chart;                   ///< 图表对象缓存
    bool m_rubberBandActive = false;   ///< 是否正在框选
    QPoint m_rubberBandStart, m_rubberBandEnd; ///< 框选起点/终点
    bool m_panning = false;            ///< 是否正在平移
    QPoint m_panStart;                 ///< 平移起点
    double m_panStartMinX = 0, m_panStartMaxX = 0, m_panStartMinY = 0, m_panStartMaxY = 0; ///< 平移开始时轴范围
    double m_zoomLevel = 1.0;          ///< 当前缩放倍率
    struct ZoomState { double minX, maxX, minY, maxY; }; ///< 缩放栈条目
    QStack<ZoomState> m_zoomStack;     ///< 缩放历史栈(undo)
    ZoomState m_originalRange;         ///< 原始数据范围(reset用)
    static constexpr double kZoomFactor = 1.5; ///< 每步缩放倍率
    static constexpr int kMinZoomLevel = 32;    ///< 最小可见采样点数
    // 统计计数器
    quint64 m_totalZooms = 0, m_totalPans = 0, m_totalResets = 0, m_totalZoomOperations = 0;
};

#endif // CHART_ZOOMCONTROLLER_H
