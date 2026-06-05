/**
 * @file EyeDiagramWidget.h
 * @brief 眼图交互显示控件 — 眼图渲染/掩模叠加/测量标注/鼠标缩放平移
 *
 * 设计: QWidget子类、接收EyeDiagramEngine渲染的QImage并叠加掩模/标注
 * 协作: EyeDiagramEngine(数据源) / EyeTypes(显示数据结构)
 *
 * 功能:
 *   - 自定义paintEvent渲染眼图QImage
 *   - 掩模多边形叠加(半透明红色)
 *   - 测量结果标注(眼高/眼宽/抖动/SNR/BER)
 *   - 鼠标滚轮缩放、拖拽平移
 */

#ifndef EYEDIAGRAMWIDGET_H
#define EYEDIAGRAMWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPointF>

#include "chart/eye/EyeTypes.h"

class EyeDiagramEngine;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QMouseEvent;

/** @brief 眼图交互显示控件 — 渲染眼图、叠加掩模与测量标注，支持鼠标缩放/平移 */
class EyeDiagramWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造眼图控件 @param parent 父控件 */
    explicit EyeDiagramWidget(QWidget* parent = nullptr);

    // ---- 引擎绑定 ----

    /** @brief 绑定眼图引擎(不获取所有权) @param engine 引擎指针 */
    void setEngine(EyeDiagramEngine* engine);

    // ---- 显示控制 ----

    /** @brief 设置掩模可见性 @param visible 是否显示 */
    void setMaskVisible(bool visible);

    /** @brief 设置测量标注可见性 @param visible 是否显示 */
    void setMeasurementVisible(bool visible);

    // ---- 统计 ----

    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const;

    /** @brief 获取累计掩模显隐切换次数 */
    quint64 totalMaskToggles() const;

    /** @brief 获取累计缩放事件次数 */
    quint64 totalZoomEvents() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

protected:
    /** @brief 自定义绘制事件 — 渲染眼图+掩模+标注 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 窗口尺寸变更事件 — 触发重绘 */
    void resizeEvent(QResizeEvent* event) override;

    /** @brief 鼠标滚轮事件 — 缩放 */
    void wheelEvent(QWheelEvent* event) override;

    /** @brief 鼠标按下事件 — 开始拖拽 */
    void mousePressEvent(QMouseEvent* event) override;

    /** @brief 鼠标移动事件 — 拖拽平移 */
    void mouseMoveEvent(QMouseEvent* event) override;

    /** @brief 鼠标释放事件 — 结束拖拽 */
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    /** @brief 引擎数据更新时刷新显示 */
    void onDiagramUpdated();

private:
    /** @brief 绘制眼图背景网格 */
    void drawGrid(QPainter& painter, const QRectF& viewport);

    /** @brief 绘制掩模多边形 */
    void drawMask(QPainter& painter, const QRectF& viewport);

    /** @brief 绘制测量标注文字 */
    void drawMeasurement(QPainter& painter, const QRectF& viewport);

    /** @brief 将数据坐标映射到像素坐标 @param dataPt 数据点 @param viewport 视口范围 @return 像素坐标 */
    QPointF dataToPixel(const QPointF& dataPt, const QRectF& viewport) const;

    /** @brief 将像素坐标映射到数据坐标 @param pixelPt 像素点 @param viewport 视口范围 @return 数据坐标 */
    QPointF pixelToData(const QPointF& pixelPt, const QRectF& viewport) const;

    EyeDiagramEngine* m_engine = nullptr;     ///< 眼图引擎(外部拥有)
    QImage m_cachedImage;                      ///< 缓存的眼图图像

    // 显示状态
    bool m_maskVisible       = true;           ///< 掩模可见性
    bool m_measurementVisible = true;          ///< 测量标注可见性
    EyeMeasurement m_lastMeasurement;          ///< 最近测量结果

    // 缩放/平移
    double m_zoomLevel = 1.0;                  ///< 缩放倍率(1.0=原始)
    QPointF m_panOffset;                       ///< 平移偏移(像素)
    bool m_dragging = false;                   ///< 是否正在拖拽
    QPointF m_dragStart;                       ///< 拖拽起点(像素)
    QPointF m_panStart;                        ///< 拖拽开始时的平移偏移

    // 统计计数器
    quint64 m_totalRepaints    = 0;            ///< 累计重绘次数
    quint64 m_totalMaskToggles = 0;            ///< 累计掩模切换次数
    quint64 m_totalZoomEvents  = 0;            ///< 累计缩放事件次数
};

#endif // EYEDIAGRAMWIDGET_H
