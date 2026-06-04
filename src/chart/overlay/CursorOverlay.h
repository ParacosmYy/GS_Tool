/**
 * @file CursorOverlay.h
 * @brief 波形游标测量叠加层 -- 在ChartWidget上绘制双游标线和差值信息
 *
 * 功能: 游标A/B(双击/右键放置竖线游标，高亮选中区域)、差值显示(ΔX/各通道ΔY/1/ΔX)、交互(鼠标拖拽)
 * 协作: ChartWidget(子控件叠加) / ChartModel(通道数据) / ZoomController(框选)
 */

#ifndef CURSOROVERLAY_H
#define CURSOROVERLAY_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QColor>

class QChart; class QChartView; class ChartModel; class ZoomController;

/** @brief 游标测量叠加层 — 叠加在ChartView上方，绘制两条竖线游标和差值信息面板 */
class CursorOverlay : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造游标叠加层 @param chartView 关联的图表视图 @param model 数据模型 @param parent 父控件指针 */
    explicit CursorOverlay(QChartView* chartView, ChartModel* model, QWidget* parent = nullptr);
    /** @brief 设置游标A的X轴坐标值(数据空间) @param x X轴数据坐标 */
    void setCursorA(double x);
    /** @brief 设置游标B的X轴坐标值(数据空间) @param x X轴数据坐标 */
    void setCursorB(double x);
    /** @brief 清除所有游标 */
    void clearCursors();
    /** @brief 游标A是否激活 @return true=游标A存在 */
    bool hasCursorA() const;
    /** @brief 游标B是否激活 @return true=游标B存在 */
    bool hasCursorB() const;
    /** @brief 获取游标A的X值 @return X轴数据坐标 */
    double cursorAX() const;
    /** @brief 获取游标B的X值 @return X轴数据坐标 */
    double cursorBX() const;
    /** @brief 设置关联的缩放控制器 @param zoom 缩放控制器指针 */
    void setZoomController(ZoomController* zoom);

    // ---- 统计计数器接口 ----
    /** @brief 获取累计游标创建次数 @return 创建计数 */
    quint64 totalCursorCreations() const;
    /** @brief 获取累计游标删除次数 @return 删除计数 */
    quint64 totalCursorDeletions() const;
    /** @brief 获取累计游标拖拽次数 @return 拖拽计数 */
    quint64 totalCursorDrags() const;
    /** @brief 获取累计游标移动次数 @return 移动计数 */
    quint64 totalCursorMoves() const;
    /** @brief 获取累计差值测量次数 @return 测量计数 */
    quint64 totalDeltaMeasurements() const;
    /** @brief 获取累计测量总次数 @return 测量总数 */
    quint64 totalMeasurements() const;
    /** @brief 获取累计峰值吸附次数 @return 吸附计数 */
    quint64 totalSnapToPeak() const;
    /** @brief 获取平均X差值 @return 平均ΔX */
    double averageDeltaX() const;
    /** @brief 获取平均Y差值 @return 平均ΔY */
    double averageDeltaY() const;
    /** @brief 重置所有游标统计计数器 */
    void resetCursorStatistics();
    /** @brief 获取累计游标显隐切换次数 @return 切换计数 */
    quint64 totalCursorToggles() const;
    /** @brief 获取累计命中测试次数 @return 测试计数 */
    quint64 totalHitTests() const;
    /** @brief 获取累计拖拽取消次数 @return 取消计数 */
    quint64 totalDragCancels() const;
    /** @brief 获取累计主题颜色变更次数 @return 变更计数 */
    quint64 totalThemeChanges() const;

protected:
    /** @brief 绘制事件处理，渲染游标线和差值信息面板 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent* event) override;
    /** @brief 事件过滤器，处理鼠标交互(拖拽/放置游标) @param watched 目标对象 @param event 事件 @return 是否拦截 */
    bool eventFilter(QObject* watched, QEvent* event) override;
    /** @brief 显示事件处理，安装事件过滤器 @param event 显示事件参数 */
    void showEvent(QShowEvent* event) override;

private:
    /** @brief 将像素X坐标转换为数据空间X值 @param pixelX 像素X坐标 @return 数据X值 */
    double pixelToDataX(int pixelX) const;
    /** @brief 将数据空间X值转换为像素X坐标 @param dataX 数据X值 @return 像素X坐标 */
    double dataToPixelX(double dataX) const;
    /** @brief 绘制单条游标竖线 @param painter 画布对象 @param pixelX 游标像素位置 @param color 游标颜色 @param label 游标标签文本 */
    void drawCursorLine(QPainter& painter, double pixelX, const QColor& color, const QString& label);
    /** @brief 绘制游标高亮选中区域 @param painter 画布对象 @param pixelAX 游标A像素位置 @param pixelBX 游标B像素位置 */
    void drawHighlightRegion(QPainter& painter, double pixelAX, double pixelBX);
    /** @brief 绘制差值信息面板 @param painter 画布对象 */
    void drawDeltaPanel(QPainter& painter);
    /** @brief 命中测试：判断像素X坐标是否在游标附近 @param pixelX 像素X坐标 @return 0=未命中, 1=游标A, 2=游标B */
    int hitTestCursor(int pixelX) const;
    /** @brief 更新差值测量统计数据 */
    void updateMeasurementStats();

    QChartView* m_chartView;
    ChartModel* m_model;
    bool m_hasCursorA = false, m_hasCursorB = false;
    double m_cursorAX = 0.0, m_cursorBX = 0.0;
    int m_draggingCursor = 0;            ///< 正在拖拽的游标(0=无, 1=A, 2=B)
    static constexpr int kHitMargin = 8;
    QColor m_cursorAColor, m_cursorBColor, m_highlightColor, m_textColor, m_panelBgColor;
    ZoomController* m_zoomController = nullptr;
    bool m_measurementDirty = false;

    // 统计计数器
    quint64 m_totalCursorCreations = 0, m_totalCursorDeletions = 0;
    quint64 m_totalCursorDrags = 0, m_totalCursorMoves = 0;
    quint64 m_totalDeltaMeasurements = 0, m_totalMeasurements = 0;
    quint64 m_totalSnapToPeak = 0, m_totalCursorToggles = 0;
    quint64 m_totalHitTests = 0, m_totalDragCancels = 0, m_totalThemeChanges = 0;
    double m_sumDeltaX = 0.0, m_sumDeltaY = 0.0;

private slots:
    void onThemeChanged();
};

#endif // CURSOROVERLAY_H
