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
    explicit CursorOverlay(QChartView* chartView, ChartModel* model, QWidget* parent = nullptr);
    void setCursorA(double x);              ///< 设置游标A的X轴坐标值(数据空间)
    void setCursorB(double x);              ///< 设置游标B的X轴坐标值(数据空间)
    void clearCursors();                    ///< 清除所有游标
    bool hasCursorA() const;                ///< 游标A是否激活
    bool hasCursorB() const;                ///< 游标B是否激活
    double cursorAX() const;                ///< 游标A的X值
    double cursorBX() const;                ///< 游标B的X值
    void setZoomController(ZoomController* zoom); ///< 设置关联的缩放控制器

    // ---- 统计计数器接口 ----
    quint64 totalCursorCreations() const;
    quint64 totalCursorDeletions() const;
    quint64 totalCursorDrags() const;
    quint64 totalCursorMoves() const;
    quint64 totalDeltaMeasurements() const;
    quint64 totalMeasurements() const;
    quint64 totalSnapToPeak() const;
    double averageDeltaX() const;
    double averageDeltaY() const;
    void resetCursorStatistics();
    quint64 totalCursorToggles() const;
    quint64 totalHitTests() const;
    quint64 totalDragCancels() const;
    quint64 totalThemeChanges() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    double pixelToDataX(int pixelX) const;
    double dataToPixelX(double dataX) const;
    void drawCursorLine(QPainter& painter, double pixelX, const QColor& color, const QString& label);
    void drawHighlightRegion(QPainter& painter, double pixelAX, double pixelBX);
    void drawDeltaPanel(QPainter& painter);
    int hitTestCursor(int pixelX) const;
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
