/**
 * @file CursorOverlay.h
 * @brief 波形游标测量叠加层 -- 在ChartWidget上绘制双游标线和差值信息
 *
 * 功能:
 *   - 游标A/B: 双击/右键放置竖线游标，高亮选中区域
 *   - 差值显示: ΔX(采样差)、各通道ΔY(值差)、1/ΔX(频率估算)
 *   - 交互: 鼠标拖拽移动游标位置
 *
 * 协作关系:
 *   - ChartWidget: 作为其子控件叠加显示
 *   - ChartModel: 读取通道数据和Y值范围
 *
 * 设计模式:
 *   - 观察者模式: 监听 ChartModel 数据变化自动重绘
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

/**
 * @brief 游标测量叠加层
 *
 * 叠加在 ChartView 上方，绘制两条竖线游标和差值信息面板。
 * 使用 QPainter 在 paintEvent 中直接绘制，不使用额外控件。
 */
class CursorOverlay : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造游标叠加层
     * @param chartView 关联的图表视图(用于坐标映射)
     * @param model 数据模型(用于读取Y值)
     * @param parent 父控件
     */
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

    /**
     * @brief 设置关联的缩放控制器(用于绘制框选矩形)
     * @param zoom 缩放控制器指针
     */
    void setZoomController(ZoomController* zoom);

    // ---- 统计计数器接口 ----

    /** @brief 获取游标移动总次数（含放置和拖拽） */
    quint64 totalCursorMoves() const;

    /** @brief 获取测量显示总次数（双游标差值面板绘制） */
    quint64 totalMeasurements() const;

    /** @brief 重置所有游标统计计数器为初始值 */
    void resetCursorStatistics();

protected:
    /** @brief 绘制游标线和差值信息面板 */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 事件过滤器 — 安装在QChartView上拦截游标交互
     *
     * 因为CursorOverlay设置WA_TransparentForMouseEvents，
     * 所有鼠标事件都穿透到QChartView。此filter在ZoomController之前
     * 安装，优先级更高。当检测到命中游标时消费事件，否则放行给ZoomController。
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

    /** @brief 首次显示时同步几何尺寸到父控件(chartView) */
    void showEvent(QShowEvent* event) override;

private:
    /**
     * @brief 从像素X坐标转换为数据空间X值
     * @param pixelX 像素坐标
     * @return 数据空间X值
     */
    double pixelToDataX(int pixelX) const;

    /**
     * @brief 从数据空间X值转换为像素X坐标
     * @param dataX 数据空间X值
     * @return 像素坐标
     */
    double dataToPixelX(double dataX) const;

    /**
     * @brief 绘制单条游标竖线
     * @param painter 画笔
     * @param pixelX 游标像素X位置
     * @param color 游标颜色
     * @param label 游标标签(A/B)
     */
    void drawCursorLine(QPainter& painter, double pixelX,
                        const QColor& color, const QString& label);

    /**
     * @brief 绘制游标间的高亮区域
     * @param painter 画笔
     * @param pixelAX 游标A像素X
     * @param pixelBX 游标B像素X
     */
    void drawHighlightRegion(QPainter& painter, double pixelAX, double pixelBX);

    /**
     * @brief 绘制差值信息面板
     * @param painter 画笔
     */
    void drawDeltaPanel(QPainter& painter);

    /**
     * @brief 判断点击位置是否在游标附近(可拖拽)
     * @param pixelX 点击像素X
     * @return 0=无, 1=游标A, 2=游标B
     */
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
    quint64 m_totalCursorMoves = 0;     ///< 游标移动总次数（含放置和拖拽）
    quint64 m_totalMeasurements = 0;    ///< 测量显示总次数（双游标差值面板绘制）

private slots:
    /**
     * @brief 主题切换时更新所有颜色成员
     *
     * 重新从ThemeManager读取语义色，刷新游标/面板/文字颜色。
     * 连接到ThemeManager::themeChanged信号。
     */
    void onThemeChanged();
};

#endif // CURSOROVERLAY_H
