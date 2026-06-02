/**
 * @file ZoomController.h
 * @brief 图表缩放/平移状态控制器
 *
 * 管理QChart的缩放级别、平移偏移和鼠标交互。
 * 支持鼠标滚轮缩放(以光标为中心)、中键拖拽平移、
 * 框选缩放(左键拖拽+Ctrl)和双击恢复原始范围。
 *
 * 设计原则:
 * - 只操作QChart的axis范围，不触碰series数据
 * - 缩放/平移后发射信号通知CursorOverlay刷新
 * - 记录zoom栈，支持无限次undo
 */

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
 *
 * 用法:
 * @code
 * auto zoom = new ZoomController(chartView, this);
 * chartView->installEventFilter(zoom);
 * @endcode
 *
 * 安装为QChartView的eventFilter后自动处理鼠标滚轮/拖拽事件。
 */
class ZoomController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造缩放控制器
     * @param chartView 关联的图表视图
     * @param parent 父对象
     */
    explicit ZoomController(QChartView* chartView, QObject* parent = nullptr);

    /** @brief 重置缩放至原始数据范围 */
    void resetZoom();

    /** @brief 当前缩放倍率(1.0=原始) */
    double zoomLevel() const;

    /** @brief 是否处于框选缩放模式 */
    bool isRubberBandActive() const;

    /** @brief 获取当前框选矩形(像素坐标，相对于chartView) */
    QRectF rubberBandRect() const;

signals:
    /** @brief 缩放/平移发生变化，接收方应刷新叠加层 */
    void viewChanged();

    /** @brief 缩放被重置到原始范围 */
    void zoomReset();

public slots:
    /** @brief 放大一步(倍率×1.5) */
    void zoomIn();

    /** @brief 缩小一步(倍率÷1.5) */
    void zoomOut();

protected:
    /** @brief 事件过滤器 — 处理滚轮/鼠标事件 */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /** @brief 保存当前axis范围到zoom栈 */
    void pushZoomState();

    /** @brief 从zoom栈恢复上一步 */
    bool popZoomState();

    /**
     * @brief 以指定像素点为中心缩放
     * @param centerPixelX 中心像素X
     * @param factor 缩放因子(>1放大, <1缩小)
     */
    void zoomAt(int centerPixelX, double factor);

    /**
     * @brief 处理鼠标滚轮缩放
     * @param event 滚轮事件
     */
    void handleWheel(QWheelEvent* event);

    /**
     * @brief 处理鼠标按下 — 开始框选或平移
     * @param event 鼠标事件
     */
    void handleMousePress(QMouseEvent* event);

    /** @brief 处理鼠标移动 — 框选绘制 */
    void handleMouseMove(QMouseEvent* event);

    /** @brief 处理鼠标释放 — 完成框选缩放 */
    void handleMouseRelease(QMouseEvent* event);

    /** @brief 处理鼠标双击 — 重置缩放 */
    void handleMouseDoubleClick(QMouseEvent* event);

    /** @brief 获取X轴对象 */
    QValueAxis* xAxis() const;

    /** @brief 获取Y轴对象 */
    QValueAxis* yAxis() const;

    // ---- 成员 ----
    QChartView* m_chartView;           ///< 关联的图表视图
    QChart* m_chart;                   ///< 图表对象缓存

    bool m_rubberBandActive = false;   ///< 是否正在框选
    QPoint m_rubberBandStart;          ///< 框选起点
    QPoint m_rubberBandEnd;            ///< 框选终点

    bool m_panning = false;            ///< 是否正在平移
    QPoint m_panStart;                 ///< 平移起点
    double m_panStartMinX = 0;         ///< 平移开始时X轴最小值
    double m_panStartMaxX = 0;         ///< 平移开始时X轴最大值
    double m_panStartMinY = 0;         ///< 平移开始时Y轴最小值
    double m_panStartMaxY = 0;         ///< 平移开始时Y轴最大值

    double m_zoomLevel = 1.0;          ///< 当前缩放倍率

    /// 缩放栈：记录每次缩放前的axis范围
    struct ZoomState {
        double minX, maxX, minY, maxY;
    };
    QStack<ZoomState> m_zoomStack;     ///< 缩放历史栈(用于undo)
    ZoomState m_originalRange;         ///< 原始数据范围(用于reset)

    static constexpr double kZoomFactor = 1.5;   ///< 每步缩放倍率
    static constexpr int kMinZoomLevel = 32;      ///< 最小可见采样点数
};

#endif // CHART_ZOOMCONTROLLER_H
