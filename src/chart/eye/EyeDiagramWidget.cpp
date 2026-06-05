/**
 * @file EyeDiagramWidget.cpp
 * @brief 眼图控件实现 — 绘制/缩放/平移/掩模叠加/测量标注
 *
 * 实现 EyeDiagramWidget 的交互功能:
 *   - paintEvent(): 眼图QImage渲染 + 网格 + 掩模 + 测量标注
 *   - wheelEvent()/mousePressEvent()/mouseMoveEvent(): 缩放平移
 *   - drawGrid()/drawMask()/drawMeasurement(): 分层绘制
 *
 * 统计接口见 EyeDiagramWidgetStats.cpp。
 */

#include "chart/eye/EyeDiagramWidget.h"
#include "chart/eye/EyeDiagramEngine.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QtMath>

// ============================================================
// 构造 / 引擎绑定
// ============================================================

/** @brief 构造眼图控件 @param parent 父控件 */
EyeDiagramWidget::EyeDiagramWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("EyeDiagramWidget"));
    setMinimumSize(200, 150);
    setMouseTracking(true);
}

/** @brief 绑定眼图引擎并连接信号 @param engine 引擎指针(不获取所有权) */
void EyeDiagramWidget::setEngine(EyeDiagramEngine* engine)
{
    if (m_engine) {
        disconnect(m_engine, &EyeDiagramEngine::diagramUpdated,
                   this, &EyeDiagramWidget::onDiagramUpdated);
    }
    m_engine = engine;
    if (m_engine) {
        connect(m_engine, &EyeDiagramEngine::diagramUpdated,
                this, &EyeDiagramWidget::onDiagramUpdated);
    }
}

// ============================================================
// 显示控制
// ============================================================

/** @brief 设置掩模可见性 @param visible 是否显示 */
void EyeDiagramWidget::setMaskVisible(bool visible)
{
    if (m_maskVisible != visible) {
        m_maskVisible = visible;
        ++m_totalMaskToggles;
        update();
    }
}

/** @brief 设置测量标注可见性 @param visible 是否显示 */
void EyeDiagramWidget::setMeasurementVisible(bool visible)
{
    if (m_measurementVisible != visible) {
        m_measurementVisible = visible;
        update();
    }
}

// ============================================================
// 绘制
// ============================================================

/** @brief 自定义绘制: 眼图图像 + 网格 + 掩模 + 测量标注 */
void EyeDiagramWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    /* 填充背景 */
    painter.fillRect(rect(), Qt::black);

    /* 计算视口(考虑缩放和平移) */
    const double invZoom = 1.0 / m_zoomLevel;
    const double vw = width() * invZoom;
    const double vh = height() * invZoom;
    const double vx = -m_panOffset.x() * invZoom - (vw - width() * invZoom) / 2.0;
    const double vy = -m_panOffset.y() * invZoom - (vh - height() * invZoom) / 2.0;
    const QRectF viewport(vx, vy, vw, vh);

    /* 绘制网格 */
    drawGrid(painter, viewport);

    /* 渲染并绘制眼图图像 */
    if (m_engine) {
        const int renderW = qMax(1, static_cast<int>(width() * m_zoomLevel));
        const int renderH = qMax(1, static_cast<int>(height() * m_zoomLevel));
        m_cachedImage = m_engine->renderEyeDiagram(renderW, renderH);
        if (!m_cachedImage.isNull()) {
            painter.drawImage(rect(), m_cachedImage);
        }

        /* 绘制掩模叠加 */
        if (m_maskVisible) {
            drawMask(painter, viewport);
        }

        /* 绘制测量标注 */
        if (m_measurementVisible) {
            m_lastMeasurement = m_engine->measure();
            drawMeasurement(painter, viewport);
        }
    }

    /* 绘制边框 */
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    ++m_totalRepaints;
}

/** @brief 绘制背景网格 @param painter 画笔 @param viewport 当前视口 */
void EyeDiagramWidget::drawGrid(QPainter& painter, const QRectF& viewport)
{
    QPen gridPen(QColor(60, 60, 60), 1, Qt::DotLine);
    painter.setPen(gridPen);

    /* 水平线(电压刻度) */
    const int hLines = 8;
    for (int i = 0; i <= hLines; ++i) {
        const int y = static_cast<int>(i * height() / hLines);
        painter.drawLine(0, y, width(), y);
    }

    /* 垂直线(时间刻度) */
    const int vLines = 10;
    for (int i = 0; i <= vLines; ++i) {
        const int x = static_cast<int>(i * width() / vLines);
        painter.drawLine(x, 0, x, height());
    }

    /* 中心十字线(高亮) */
    QPen centerPen(QColor(100, 100, 100), 1, Qt::DashLine);
    painter.setPen(centerPen);
    painter.drawLine(width() / 2, 0, width() / 2, height());
    painter.drawLine(0, height() / 2, width(), height() / 2);
}

/** @brief 绘制掩模多边形(半透明红色) @param painter 画笔 @param viewport 当前视口 */
void EyeDiagramWidget::drawMask(QPainter& painter, const QRectF& viewport)
{
    if (!m_engine) {
        return;
    }
    const EyeMask mask = m_engine->mask();
    if (mask.upperBoundary.isEmpty() && mask.lowerBoundary.isEmpty()) {
        return;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0, 60));

    /* 上边界多边形 */
    if (mask.upperBoundary.size() >= 2) {
        QPolygonF upperPoly;
        for (const auto& pt : mask.upperBoundary) {
            upperPoly.append(dataToPixel(pt, viewport));
        }
        /* 闭合到底部 */
        upperPoly.append(dataToPixel(
            QPointF(mask.upperBoundary.last().x(), viewport.top()), viewport));
        upperPoly.append(dataToPixel(
            QPointF(mask.upperBoundary.first().x(), viewport.top()), viewport));
        painter.drawPolygon(upperPoly);
    }

    /* 下边界多边形 */
    if (mask.lowerBoundary.size() >= 2) {
        QPolygonF lowerPoly;
        for (const auto& pt : mask.lowerBoundary) {
            lowerPoly.append(dataToPixel(pt, viewport));
        }
        lowerPoly.append(dataToPixel(
            QPointF(mask.lowerBoundary.last().x(), viewport.bottom()), viewport));
        lowerPoly.append(dataToPixel(
            QPointF(mask.lowerBoundary.first().x(), viewport.bottom()), viewport));
        painter.drawPolygon(lowerPoly);
    }

    /* 掩模边框线 */
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 80, 80, 180), 1.5));
    if (mask.upperBoundary.size() >= 2) {
        QPolygonF upperLine;
        for (const auto& pt : mask.upperBoundary) {
            upperLine.append(dataToPixel(pt, viewport));
        }
        painter.drawPolyline(upperLine);
    }
    if (mask.lowerBoundary.size() >= 2) {
        QPolygonF lowerLine;
        for (const auto& pt : mask.lowerBoundary) {
            lowerLine.append(dataToPixel(pt, viewport));
        }
        painter.drawPolyline(lowerLine);
    }
}

/** @brief 绘制测量标注文字 @param painter 画笔 @param viewport 当前视口 */
void EyeDiagramWidget::drawMeasurement(QPainter& painter, const QRectF& viewport)
{
    Q_UNUSED(viewport)

    const EyeMeasurement& m = m_lastMeasurement;
    if (m.eyeHeight <= 0.0 && m.eyeWidth <= 0.0) {
        return;
    }

    /* 半透明背景面板 */
    const int panelW = 200;
    const int panelH = 130;
    const int margin = 8;
    QRect panelRect(width() - panelW - margin, margin, panelW, panelH);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.drawRoundedRect(panelRect, 6, 6);

    /* 文字标注 */
    painter.setPen(QColor(200, 220, 255));
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);

    const int lineH = 16;
    int y = panelRect.top() + 14;
    const int x = panelRect.left() + 10;

    auto drawLine = [&](const QString& label, const QString& value) {
        painter.drawText(x, y, label);
        painter.drawText(x + 100, y, value);
        y += lineH;
    };

    drawLine(tr("Eye Height"), QString::number(m.eyeHeight, 'f', 4) + QStringLiteral(" V"));
    drawLine(tr("Eye Width"),  QString::number(m.eyeWidth * 1e9, 'f', 2) + QStringLiteral(" ns"));
    drawLine(tr("Jitter RMS"),  QString::number(m.jitterRms * 1e12, 'f', 2) + QStringLiteral(" ps"));
    drawLine(tr("Jitter P-P"),  QString::number(m.jitterPp * 1e12, 'f', 2) + QStringLiteral(" ps"));
    drawLine(tr("SNR"),         QString::number(m.signalToNoiseRatio, 'f', 1) + QStringLiteral(" dB"));
    drawLine(tr("Q Factor"),    QString::number(m.qualityFactor, 'f', 2));
    drawLine(tr("BER"),         QString::number(m.bitErrorRate, 'e', 2));
}

// ============================================================
// 坐标映射
// ============================================================

/** @brief 数据坐标→像素坐标 @param dataPt 数据点 @param viewport 视口 @return 像素坐标 */
QPointF EyeDiagramWidget::dataToPixel(const QPointF& dataPt,
                                       const QRectF& viewport) const
{
    const double px = (dataPt.x() - viewport.left()) / viewport.width() * width();
    const double py = (1.0 - (dataPt.y() - viewport.top()) / viewport.height()) * height();
    return QPointF(px, py);
}

/** @brief 像素坐标→数据坐标 @param pixelPt 像素点 @param viewport 视口 @return 数据坐标 */
QPointF EyeDiagramWidget::pixelToData(const QPointF& pixelPt,
                                       const QRectF& viewport) const
{
    const double dx = viewport.left() + pixelPt.x() / width() * viewport.width();
    const double dy = viewport.top() + (1.0 - pixelPt.y() / height()) * viewport.height();
    return QPointF(dx, dy);
}

// ============================================================
// 事件处理
// ============================================================

/** @brief 窗口尺寸变更 — 触发重绘 */
void EyeDiagramWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

/** @brief 鼠标滚轮缩放 */
void EyeDiagramWidget::wheelEvent(QWheelEvent* event)
{
    const double factor = (event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    m_zoomLevel = qBound(0.25, m_zoomLevel * factor, 16.0);
    ++m_totalZoomEvents;
    update();
}

/** @brief 鼠标按下 — 开始拖拽 */
void EyeDiagramWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = event->position();
        m_panStart = m_panOffset;
        setCursor(Qt::ClosedHandCursor);
    }
}

/** @brief 鼠标移动 — 拖拽平移 */
void EyeDiagramWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        const QPointF delta = event->position() - m_dragStart;
        m_panOffset = m_panStart + delta;
        update();
    }
}

/** @brief 鼠标释放 — 结束拖拽 */
void EyeDiagramWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

// ============================================================
// 槽函数
// ============================================================

/** @brief 引擎数据更新 — 刷新显示 */
void EyeDiagramWidget::onDiagramUpdated()
{
    update();
}
