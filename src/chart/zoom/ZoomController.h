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
    /** @brief 构造缩放控制器 @param chartView 关联的图表视图 @param parent 父对象 */
    explicit ZoomController(QChartView* chartView, QObject* parent = nullptr);
    /** @brief 重置缩放至原始数据范围 */
    void resetZoom();
    /** @brief 获取当前缩放倍率 @return 倍率(1.0=原始) */
    double zoomLevel() const;
    /** @brief 查询是否框选模式 @return true=正在框选 */
    bool isRubberBandActive() const;
    /** @brief 获取当前框选矩形(像素坐标) @return 框选区域 */
    QRectF rubberBandRect() const;
    // ---- 统计计数器接口 ----
    /** @brief 获取缩放操作总次数(含滚轮/框选/按钮) @return 缩放总次数 */
    quint64 totalZooms() const;
    /** @brief 获取平移操作总次数 @return 平移总次数 */
    quint64 totalPans() const;
    /** @brief 获取缩放重置总次数 @return 重置总次数 */
    quint64 totalResets() const;
    /** @brief 获取所有缩放操作次数(含缩放/平移/重置/undo) @return 操作总次数 */
    quint64 totalZoomOperations() const { return m_totalZoomOperations; }
    /** @brief 获取累计撤销(popZoomState)次数 @return 撤销次数 */
    quint64 totalUndos() const { return m_totalUndos; }
    /** @brief 重置所有缩放统计计数器 */
    void resetZoomStatistics();

signals:
    /** @brief 缩放/平移变化，接收方应刷新叠加层 */
    void viewChanged();
    /** @brief 缩放被重置到原始范围 */
    void zoomReset();

public slots:
    /** @brief 放大一步(倍率x1.5) */
    void zoomIn();
    /** @brief 缩小一步(倍率/1.5) */
    void zoomOut();

protected:
    /** @brief 事件过滤器，处理鼠标滚轮/拖拽/双击事件 @param watched 目标对象 @param event 事件 @return 是否拦截 */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /** @brief 保存当前axis范围到zoom栈 */
    void pushZoomState();
    /** @brief 从zoom栈恢复上一步 @return true=恢复成功 */
    bool popZoomState();
    /** @brief 以指定像素点为中心缩放 @param centerPixelX 中心X @param factor 缩放因子 */
    void zoomAt(int centerPixelX, double factor);
    /** @brief 处理滚轮缩放事件 @param event 滚轮事件 */
    void handleWheel(QWheelEvent* event);
    /** @brief 处理鼠标按下(开始框选或平移) @param event 鼠标事件 */
    void handleMousePress(QMouseEvent* event);
    /** @brief 处理鼠标移动(框选绘制) @param event 鼠标事件 */
    void handleMouseMove(QMouseEvent* event);
    /** @brief 处理鼠标释放(完成框选缩放) @param event 鼠标事件 */
    void handleMouseRelease(QMouseEvent* event);
    /** @brief 处理双击(重置缩放) @param event 鼠标事件 */
    void handleMouseDoubleClick(QMouseEvent* event);
    /** @brief 获取X轴对象 @return X轴指针 */
    QValueAxis* xAxis() const;
    /** @brief 获取Y轴对象 @return Y轴指针 */
    QValueAxis* yAxis() const;
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
    quint64 m_totalUndos = 0; ///< 累计撤销(popZoomState成功恢复)次数
};

#endif // CHART_ZOOMCONTROLLER_H
