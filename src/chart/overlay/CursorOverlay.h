/**
 * @file CursorOverlay.h
 * @brief 波形游标测量叠加层 -- 在ChartWidget上绘制双游标线和差值信息
 *
 * 功能: 游标A/B(双击/右键放置竖线游标，高亮选中区域)、差值显示(ΔX/各通道ΔY/1/ΔX)、交互(鼠标拖拽移动游标)
 * 协作: ChartWidget(作为子控件叠加显示) / ChartModel(读取通道数据和Y值范围)
 * 设计: 观察者模式 — 监听ChartModel数据变化自动重绘
 */

#ifndef CURSOROVERLAY_H
#define CURSOROVERLAY_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QColor>

class QChart;
class QChartView;
class ChartModel;
class ZoomController;

/** @brief 游标测量叠加层 — 叠加在ChartView上方，绘制两条竖线游标和差值信息面板，使用QPainter直接绘制 */
class CursorOverlay : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造游标叠加层 @param chartView 关联的图表视图 @param model 数据模型 @param parent 父控件 */
    explicit CursorOverlay(QChartView* chartView, ChartModel* model,
                           QWidget* parent = nullptr);

    /** @brief 设置游标A的X轴坐标值(数据空间) @param x X轴值 */
    void setCursorA(double x);

    /** @brief 设置游标B的X轴坐标值(数据空间) @param x X轴值 */
    void setCursorB(double x);

    /** @brief 清除所有游标 */
    void clearCursors();

    /** @brief 游标A是否激活 */
    bool hasCursorA() const;

    /** @brief 游标B是否激活 */
    bool hasCursorB() const;

    /** @brief 游标A的X值 */
    double cursorAX() const;

    /** @brief 游标B的X值 */
    double cursorBX() const;

    /** @brief 设置关联的缩放控制器(用于绘制框选矩形) @param zoom 缩放控制器指针 */
    void setZoomController(ZoomController* zoom);

    // ---- 统计计数器接口 ----

    /** @brief 获取游标创建总次数（双击/右键放置游标） */
    quint64 totalCursorCreations() const;

    /** @brief 获取游标删除总次数（clearCursors调用次数） */
    quint64 totalCursorDeletions() const;

    /** @brief 获取游标拖拽移动总次数（鼠标拖拽游标） */
    quint64 totalCursorDrags() const;

    /** @brief 获取游标移动总次数（含放置和拖拽） */
    quint64 totalCursorMoves() const;

    /** @brief 获取差值测量总次数（双游标差值面板绘制） */
    quint64 totalDeltaMeasurements() const;

    /** @brief 获取测量显示总次数（双游标差值面板绘制，兼容旧接口） */
    quint64 totalMeasurements() const;

    /** @brief 获取峰值吸附总次数（游标自动吸附到最近峰值点） */
    quint64 totalSnapToPeak() const;

    /** @brief 获取历次测量的平均ΔX值 @return 平均ΔX */
    double averageDeltaX() const;

    /** @brief 获取历次测量的平均ΔY值(首个通道) @return 平均ΔY */
    double averageDeltaY() const;

    /** @brief 重置所有游标统计计数器为初始值 */
    void resetCursorStatistics();

protected:
    /** @brief 绘制游标线和差值信息面板 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 事件过滤器 — 拦截QChartView游标交互(双击/右键放置，左键拖拽) */
    bool eventFilter(QObject* watched, QEvent* event) override;

    /** @brief 首次显示时同步几何尺寸到父控件(chartView) */
    void showEvent(QShowEvent* event) override;

private:
    /** @brief 从像素X坐标转换为数据空间X值 @param pixelX 像素坐标 @return 数据空间X值 */
    double pixelToDataX(int pixelX) const;

    /** @brief 从数据空间X值转换为像素X坐标 @param dataX 数据空间X值 @return 像素坐标 */
    double dataToPixelX(double dataX) const;

    /** @brief 绘制单条游标竖线 @param painter 画笔 @param pixelX 游标像素X @param color 颜色 @param label 标签 */
    void drawCursorLine(QPainter& painter, double pixelX,
                        const QColor& color, const QString& label);

    /** @brief 绘制游标间的高亮区域 @param painter 画笔 @param pixelAX 游标A像素X @param pixelBX 游标B像素X */
    void drawHighlightRegion(QPainter& painter, double pixelAX, double pixelBX);

    /** @brief 绘制差值信息面板 @param painter 画笔 */
    void drawDeltaPanel(QPainter& painter);

    /** @brief 判断点击位置是否在游标附近 @param pixelX 点击像素X @return 0=无, 1=游标A, 2=游标B */
    int hitTestCursor(int pixelX) const;

    QChartView* m_chartView;        ///< 关联的图表视图
    ChartModel* m_model;            ///< 数据模型

    bool m_hasCursorA = false;      ///< 游标A是否激活
    bool m_hasCursorB = false;      ///< 游标B是否激活
    double m_cursorAX = 0.0;        ///< 游标A的X数据值
    double m_cursorBX = 0.0;        ///< 游标B的X数据值

    int m_draggingCursor = 0;       ///< 正在拖拽的游标(0=无, 1=A, 2=B)
    static constexpr int kHitMargin = 8; ///< 游标拖拽命中检测范围(像素)

    // ---- 颜色(从ThemeManager获取) ----
    QColor m_cursorAColor;          ///< 游标A颜色
    QColor m_cursorBColor;          ///< 游标B颜色
    QColor m_highlightColor;        ///< 高亮区域颜色
    QColor m_textColor;             ///< 差值面板文字颜色
    QColor m_panelBgColor;          ///< 差值面板背景色

    ZoomController* m_zoomController = nullptr; ///< 关联的缩放控制器(绘制框选用)

    // 统计计数器
    quint64 m_totalCursorCreations = 0;     ///< 游标创建总次数
    quint64 m_totalCursorDeletions = 0;     ///< 游标删除总次数
    quint64 m_totalCursorDrags = 0;         ///< 游标拖拽移动总次数
    quint64 m_totalCursorMoves = 0;         ///< 游标移动总次数
    quint64 m_totalDeltaMeasurements = 0;   ///< 差值测量总次数
    quint64 m_totalMeasurements = 0;        ///< 测量显示总次数（兼容旧接口）
    quint64 m_totalSnapToPeak = 0;          ///< 峰值吸附总次数
    double m_sumDeltaX = 0.0;               ///< 累计ΔX值(用于计算averageDeltaX)
    double m_sumDeltaY = 0.0;               ///< 累计ΔY值(首个通道，用于计算averageDeltaY)

private slots:
    /** @brief 主题切换时更新所有颜色成员 */
    void onThemeChanged();
};

#endif // CURSOROVERLAY_H
