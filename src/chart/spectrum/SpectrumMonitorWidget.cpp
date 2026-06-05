/**
 * @file SpectrumMonitorWidget.cpp
 * @brief 频谱监控显示控件核心实现 — QPainter瀑布图/频谱曲线/网格/峰值标记
 *
 * 从 SpectrumMonitorWidget.cpp 拆分，包含全部绘制逻辑。
 * 统计 getter 见 SpectrumMonitorWidgetStats.cpp。
 */

#include "chart/spectrum/SpectrumMonitorWidget.h"
#include "chart/spectrum/SpectrumMonitor.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtMath>
#include <algorithm>

// ============================================================
// 构造
// ============================================================

/** @brief 构造频谱监控控件 */
SpectrumMonitorWidget::SpectrumMonitorWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("SpectrumMonitorWidget"));
    setMinimumSize(320, 240);
}

// ============================================================
// 引擎绑定
// ============================================================

/** @brief 绑定频谱监控引擎 */
void SpectrumMonitorWidget::setMonitor(SpectrumMonitor* monitor)
{
    if (m_monitor) {
        disconnect(m_monitor, &SpectrumMonitor::spectrumUpdated,
                   this, &SpectrumMonitorWidget::onSpectrumUpdated);
        disconnect(m_monitor, &SpectrumMonitor::spectrogramUpdated,
                   this, &SpectrumMonitorWidget::onSpectrogramUpdated);
    }
    m_monitor = monitor;
    if (m_monitor) {
        connect(m_monitor, &SpectrumMonitor::spectrumUpdated,
                this, &SpectrumMonitorWidget::onSpectrumUpdated);
        connect(m_monitor, &SpectrumMonitor::spectrogramUpdated,
                this, &SpectrumMonitorWidget::onSpectrogramUpdated);
    }
}

// ============================================================
// 显示控制
// ============================================================

/** @brief 设置峰值保持曲线可见性 */
void SpectrumMonitorWidget::setPeakHoldVisible(bool visible)
{
    m_peakHoldVisible = visible;
    m_totalPeakHoldToggles++;
    update();
}

/** @brief 设置瀑布图可见性 */
void SpectrumMonitorWidget::setSpectrogramVisible(bool visible)
{
    m_spectrogramVisible = visible;
    update();
}

/** @brief 设置网格可见性 */
void SpectrumMonitorWidget::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    update();
}

/** @brief 设置dBFS显示范围 */
void SpectrumMonitorWidget::setDbRange(double minDb, double maxDb)
{
    m_minDb = minDb;
    m_maxDb = maxDb;
    update();
}

/** @brief 设置频率显示范围(Hz) */
void SpectrumMonitorWidget::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = minFreq;
    m_maxFreq = maxFreq;
    update();
}

/** @brief 设置色图方案名称 */
void SpectrumMonitorWidget::setColormap(const QString& colormap)
{
    m_colormap = colormap;
    update();
}

// ============================================================
// Qt事件
// ============================================================

/** @brief 自定义绘制事件 */
void SpectrumMonitorWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    m_totalRepaints++;
    const int w = width();
    const int h = height();

    // 背景
    painter.fillRect(rect(), QColor(20, 20, 30));

    // 布局计算
    const int specH = m_spectrogramVisible
                          ? static_cast<int>(h * m_spectrumRatio) : h;
    const int sgH = h - specH;

    QRect specRect(0, 0, w, specH);
    QRect sgRect(0, specH, w, sgH);

    // 绘制频谱
    drawSpectrum(painter, specRect);

    // 绘制瀑布图
    if (m_spectrogramVisible && sgH > 0) {
        drawSpectrogram(painter, sgRect);
    }

    // 绘制网格
    if (m_gridVisible) {
        drawGrid(painter, specRect, sgRect);
    }
}

/** @brief 窗口尺寸变更事件 */
void SpectrumMonitorWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    rebuildSpectrogramImage();
}

// ============================================================
// Slots
// ============================================================

/** @brief 频谱更新时刷新显示 */
void SpectrumMonitorWidget::onSpectrumUpdated(const SpectrumSlice& slice)
{
    m_currentSlice = slice;
    if (m_monitor) {
        m_peakHoldMags = m_monitor->peakHoldMagnitudes();
    }
    update();
}

/** @brief 瀑布图更新时重建缓存 */
void SpectrumMonitorWidget::onSpectrogramUpdated()
{
    rebuildSpectrogramImage();
}

// ============================================================
// 绘制: 频谱曲线
// ============================================================

/** @brief 绘制频谱曲线区域 */
void SpectrumMonitorWidget::drawSpectrum(QPainter& painter, const QRect& rect)
{
    if (m_currentSlice.freqs.isEmpty()) return;

    const auto& freqs = m_currentSlice.freqs;
    const auto& mags  = m_currentSlice.magnitudes;
    const int n = qMin(freqs.size(), mags.size());
    if (n == 0) return;

    const double dbRange = m_maxDb - m_minDb;
    if (dbRange <= 0.0) return;

    // 频谱曲线
    QPolygonF poly;
    for (int i = 0; i < n; ++i) {
        double f = freqs[i];
        if (f < m_minFreq || f > m_maxFreq) continue;
        double x = rect.x() + (f - m_minFreq) / (m_maxFreq - m_minFreq) * rect.width();
        double dbNorm = (mags[i] - m_minDb) / dbRange;
        dbNorm = qBound(0.0, dbNorm, 1.0);
        double y = rect.y() + rect.height() - dbNorm * rect.height();
        poly.append(QPointF(x, y));
    }

    // 填充渐变
    if (poly.size() > 1) {
        QLinearGradient grad(0, rect.y(), 0, rect.y() + rect.height());
        grad.setColorAt(0.0, QColor(0, 180, 255, 120));
        grad.setColorAt(1.0, QColor(0, 80, 180, 20));
        QPen fillPen(Qt::NoPen);
        painter.setPen(fillPen);
        painter.setBrush(QBrush(grad));

        QPolygonF fillPoly = poly;
        fillPoly.append(QPointF(poly.last().x(), rect.y() + rect.height()));
        fillPoly.append(QPointF(poly.first().x(), rect.y() + rect.height()));
        painter.drawPolygon(fillPoly);
    }

    // 频谱线条
    QPen curvePen(QColor(0, 200, 255), 1.5);
    painter.setPen(curvePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPolyline(poly);

    // 峰值保持曲线
    if (m_peakHoldVisible && !m_peakHoldMags.isEmpty()) {
        QPolygonF peakPoly;
        for (int i = 0; i < qMin(n, m_peakHoldMags.size()); ++i) {
            double f = freqs[i];
            if (f < m_minFreq || f > m_maxFreq) continue;
            double x = rect.x() + (f - m_minFreq) / (m_maxFreq - m_minFreq) * rect.width();
            double dbNorm = (m_peakHoldMags[i] - m_minDb) / dbRange;
            dbNorm = qBound(0.0, dbNorm, 1.0);
            double y = rect.y() + rect.height() - dbNorm * rect.height();
            peakPoly.append(QPointF(x, y));
        }
        QPen peakPen(QColor(255, 100, 50, 180), 1.0, Qt::DashLine);
        painter.setPen(peakPen);
        painter.drawPolyline(peakPoly);
    }

    // 峰值标记
    drawPeakMarker(painter, rect);
}

// ============================================================
// 绘制: 瀑布图
// ============================================================

/** @brief 绘制瀑布图区域 */
void SpectrumMonitorWidget::drawSpectrogram(QPainter& painter, const QRect& rect)
{
    if (!m_monitor || m_spectrogramImage.isNull()) {
        painter.fillRect(rect, QColor(15, 15, 25));
        return;
    }

    // 缩放图像到矩形区域
    QImage scaled = m_spectrogramImage.scaled(
        rect.width(), rect.height(),
        Qt::IgnoreAspectRatio, Qt::FastTransformation);
    painter.drawImage(rect.topLeft(), scaled);
}

// ============================================================
// 绘制: 网格
// ============================================================

/** @brief 绘制网格叠加层 */
void SpectrumMonitorWidget::drawGrid(QPainter& painter,
                                     const QRect& specRect,
                                     const QRect& sgRect)
{
    QPen gridPen(QColor(255, 255, 255, 40), 1);
    QPen textPen(QColor(200, 200, 200, 160));
    painter.setPen(gridPen);
    QFont font = painter.font();
    font.setPixelSize(10);
    painter.setFont(font);

    const double dbRange = m_maxDb - m_minDb;
    const double freqRange = m_maxFreq - m_minFreq;

    // 频率轴网格线(竖线) — 每1kHz/5kHz/10kHz
    double freqStep = 1000.0;
    if (freqRange > 100000.0) freqStep = 10000.0;
    if (freqRange < 5000.0) freqStep = 500.0;

    for (double f = qCeil(m_minFreq / freqStep) * freqStep;
         f <= m_maxFreq; f += freqStep) {
        double x = specRect.x() + (f - m_minFreq) / freqRange * specRect.width();
        painter.drawLine(static_cast<int>(x), specRect.y(),
                         static_cast<int>(x), sgRect.isEmpty()
                             ? specRect.bottom() : sgRect.bottom());
        painter.setPen(textPen);
        QString label;
        if (f >= 1000.0) {
            label = QStringLiteral("%1k").arg(f / 1000.0, 0, 'f', (f < 10000.0) ? 1 : 0);
        } else {
            label = QString::number(static_cast<int>(f));
        }
        painter.drawText(static_cast<int>(x) + 2, specRect.bottom() - 2, label);
        painter.setPen(gridPen);
    }

    // dB轴网格线(横线) — 每20dB
    for (double db = m_minDb; db <= m_maxDb; db += 20.0) {
        double dbNorm = (db - m_minDb) / dbRange;
        double y = specRect.y() + specRect.height() - dbNorm * specRect.height();
        painter.drawLine(specRect.x(), static_cast<int>(y),
                         specRect.right(), static_cast<int>(y));
        painter.setPen(textPen);
        painter.drawText(specRect.x() + 2, static_cast<int>(y) - 2,
                         QString::number(static_cast<int>(db)));
        painter.setPen(gridPen);
    }
}

// ============================================================
// 绘制: 峰值标记
// ============================================================

/** @brief 绘制峰值标记 */
void SpectrumMonitorWidget::drawPeakMarker(QPainter& painter, const QRect& rect)
{
    if (m_currentSlice.magnitudes.isEmpty()) return;

    const auto& mags = m_currentSlice.magnitudes;
    const auto& freqs = m_currentSlice.freqs;
    const int n = qMin(mags.size(), freqs.size());
    if (n == 0) return;

    // 找峰值
    int peakIdx = 0;
    double peakVal = mags[0];
    for (int i = 1; i < n; ++i) {
        if (mags[i] > peakVal) { peakVal = mags[i]; peakIdx = i; }
    }

    double f = freqs[peakIdx];
    if (f < m_minFreq || f > m_maxFreq) return;

    double dbRange = m_maxDb - m_minDb;
    double x = rect.x() + (f - m_minFreq) / (m_maxFreq - m_minFreq) * rect.width();
    double dbNorm = (peakVal - m_minDb) / dbRange;
    dbNorm = qBound(0.0, dbNorm, 1.0);
    double y = rect.y() + rect.height() - dbNorm * rect.height();

    // 三角标记
    QPen markerPen(QColor(255, 220, 50), 2);
    painter.setPen(markerPen);
    QPolygonF tri;
    tri << QPointF(x, y - 6) << QPointF(x - 4, y - 12) << QPointF(x + 4, y - 12);
    painter.drawPolygon(tri);

    // 标注文字
    QFont font = painter.font();
    font.setPixelSize(10);
    painter.setFont(font);
    QString label;
    if (f >= 1000.0) {
        label = QStringLiteral("%1kHz %2dB").arg(f / 1000.0, 0, 'f', 1).arg(peakVal, 0, 'f', 1);
    } else {
        label = QStringLiteral("%1Hz %2dB").arg(f, 0, 'f', 0).arg(peakVal, 0, 'f', 1);
    }
    painter.drawText(static_cast<int>(x) + 6, static_cast<int>(y) - 6, label);
}

// ============================================================
// 颜色映射
// ============================================================

/** @brief 将归一化dB值映射为颜色 */
QColor SpectrumMonitorWidget::magnitudeColor(double normalized) const
{
    normalized = qBound(0.0, normalized, 1.0);

    if (m_colormap == QStringLiteral("Viridis")) {
        double r = qBound(0, static_cast<int>(255 * (0.267 + normalized * 0.329)), 255);
        double g = qBound(0, static_cast<int>(255 * (0.004 + normalized * 0.882)), 255);
        double b = qBound(0, static_cast<int>(255 * (0.329 + normalized * 0.339)), 255);
        return QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
    } else if (m_colormap == QStringLiteral("Plasma")) {
        double r = qBound(0, static_cast<int>(255 * (0.050 + normalized * 0.897)), 255);
        double g = qBound(0, static_cast<int>(255 * (normalized * 0.694)), 255);
        double b = qBound(0, static_cast<int>(255 * (0.528 + normalized * 0.472 * (1 - normalized))), 255);
        return QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
    } else if (m_colormap == QStringLiteral("Jet")) {
        // Jet: 蓝→青→绿→黄→红
        double r = qBound(0.0, qMin(1.0, 1.5 - qAbs(normalized - 0.75) * 4.0), 1.0);
        double g = qBound(0.0, qMin(1.0, 1.5 - qAbs(normalized - 0.5) * 4.0), 1.0);
        double b = qBound(0.0, qMin(1.0, 1.5 - qAbs(normalized - 0.25) * 4.0), 1.0);
        return QColor(static_cast<int>(r * 255), static_cast<int>(g * 255),
                      static_cast<int>(b * 255));
    }
    // Default: Inferno (黑→紫→红→黄→白)
    if (normalized < 0.25) {
        double t = normalized / 0.25;
        return QColor(static_cast<int>(t * 80), 0, static_cast<int>(40 + t * 100));
    } else if (normalized < 0.5) {
        double t = (normalized - 0.25) / 0.25;
        return QColor(static_cast<int>(80 + t * 175), static_cast<int>(t * 30),
                      static_cast<int>(140 - t * 60));
    } else if (normalized < 0.75) {
        double t = (normalized - 0.5) / 0.25;
        return QColor(static_cast<int>(255), static_cast<int>(30 + t * 170),
                      static_cast<int>(80 - t * 80));
    } else {
        double t = (normalized - 0.75) / 0.25;
        return QColor(255, static_cast<int>(200 + t * 55), static_cast<int>(t * 180));
    }
}

// ============================================================
// 瀑布图缓存重建
// ============================================================

/** @brief 构建瀑布图QImage缓存 */
void SpectrumMonitorWidget::rebuildSpectrogramImage()
{
    if (!m_monitor) return;

    auto sg = m_monitor->spectrogram();
    if (sg.isEmpty()) {
        m_spectrogramImage = QImage();
        return;
    }

    const int rows = sg.size();
    const int cols = sg.isEmpty() ? 0 : sg[0].size();
    if (cols == 0) return;

    m_spectrogramImage = QImage(cols, rows, QImage::Format_RGB32);
    const double dbRange = m_maxDb - m_minDb;

    for (int r = 0; r < rows; ++r) {
        const auto& row = sg[r];
        for (int c = 0; c < qMin(cols, row.size()); ++c) {
            double dbNorm = (row[c] - m_minDb) / (dbRange > 0.0 ? dbRange : 1.0);
            dbNorm = qBound(0.0, dbNorm, 1.0);
            m_spectrogramImage.setPixelColor(c, r, magnitudeColor(dbNorm));
        }
    }
}
