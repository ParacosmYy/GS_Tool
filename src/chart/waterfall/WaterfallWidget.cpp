#include "chart/waterfall/WaterfallWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QLinearGradient>

WaterfallWidget::WaterfallWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("WaterfallWidget");
    setMouseTracking(true);
    setMinimumSize(200, 150);
    connect(&m_scrollTimer, &QTimer::timeout, this, &WaterfallWidget::scrollImage);
    m_scrollTimer.setInterval(m_scrollSpeed);
}

WaterfallWidget::~WaterfallWidget() = default;

void WaterfallWidget::addSpectrum(const QVector<double> &spectrum)
{
    if (m_paused) return;
    m_history.append(spectrum);
    if (m_history.size() > m_maxLines) {
        m_history.removeFirst();
    }
    m_currentLine = m_history.size() - 1;

    // Draw new line at bottom
    if (m_waterfall.isNull()) {
        m_waterfall = QPixmap(width(), m_maxLines);
        m_waterfall.fill(Qt::black);
    }
    QPainter p(&m_waterfall);
    int y = m_currentLine % m_maxLines;
    double range = m_maxValue - m_minValue;
    if (range <= 0) range = 1.0;
    int binW = qMax(1, width() / spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i) {
        QColor c = valueToColor(spectrum[i]);
        p.setPen(c);
        p.drawLine(i * binW, y, (i + 1) * binW, y);
    }
    emit spectrumAdded(m_history.size());
    update();
}

void WaterfallWidget::setMaxLines(int lines)
{
    m_maxLines = qMax(10, lines);
    while (m_history.size() > m_maxLines) m_history.removeFirst();
    m_waterfall = QPixmap();
    update();
}

void WaterfallWidget::setColorRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
}

void WaterfallWidget::setScrollSpeed(int ms)
{
    m_scrollSpeed = qMax(10, ms);
    m_scrollTimer.setInterval(m_scrollSpeed);
}

void WaterfallWidget::clear()
{
    m_history.clear();
    m_currentLine = 0;
    m_waterfall = QPixmap();
    update();
}

void WaterfallWidget::pause() { m_paused = true; m_scrollTimer.stop(); }
void WaterfallWidget::resume() { m_paused = false; m_scrollTimer.start(); }

void WaterfallWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    if (m_waterfall.isNull()) {
        p.fillRect(rect(), Qt::black);
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, tr("No data"));
        return;
    }
    p.drawPixmap(0, 0, m_waterfall.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

void WaterfallWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_waterfall = QPixmap();
}

void WaterfallWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    int col = x * (m_history.isEmpty() ? 1 : m_history[0].size()) / width();
    int row = y * m_maxLines / height();
    if (row >= 0 && row < m_history.size() && col >= 0 && col < m_history[row].size()) {
        emit valueAtCursor(col, m_history[row][col]);
    }
}

void WaterfallWidget::scrollImage()
{
    update();
}

QColor WaterfallWidget::valueToColor(double value) const
{
    double t = (value - m_minValue) / (m_maxValue - m_minValue);
    t = qBound(0.0, t, 1.0);
    // Cool-to-hot: black -> blue -> cyan -> green -> yellow -> red
    int r, g, b;
    if (t < 0.25) {
        r = 0; g = 0; b = static_cast<int>(t * 4 * 255);
    } else if (t < 0.5) {
        r = 0; g = static_cast<int>((t - 0.25) * 4 * 255); b = static_cast<int>((0.5 - t) * 4 * 255);
    } else if (t < 0.75) {
        r = static_cast<int>((t - 0.5) * 4 * 255); g = 255; b = 0;
    } else {
        r = 255; g = static_cast<int>((1.0 - t) * 4 * 255); b = 0;
    }
    return QColor(r, g, b);
}