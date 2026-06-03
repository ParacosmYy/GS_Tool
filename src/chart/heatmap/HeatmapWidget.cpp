#include "chart/heatmap/HeatmapWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <cmath>

HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HeatmapWidget");
    setMouseTracking(true);
    setMinimumSize(100, 100);
}

HeatmapWidget::~HeatmapWidget() = default;

void HeatmapWidget::setData(const QVector<QVector<double>> &data)
{
    m_data = data;
    m_dirty = true;
    if (m_autoScale) {
        m_minValue = std::numeric_limits<double>::max();
        m_maxValue = std::numeric_limits<double>::lowest();
        for (const auto &row : m_data) {
            for (double v : row) {
                if (v < m_minValue) m_minValue = v;
                if (v > m_maxValue) m_maxValue = v;
            }
        }
        if (m_minValue >= m_maxValue) { m_minValue = 0; m_maxValue = 1; }
    }
    update();
}

void HeatmapWidget::setColorRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    m_autoScale = false;
    m_dirty = true;
    update();
}

void HeatmapWidget::setCellSize(int size)
{
    m_cellSize = qMax(4, size);
    m_dirty = true;
    update();
}

void HeatmapWidget::setShowValues(bool show)
{
    m_showValues = show;
    m_dirty = true;
    update();
}

void HeatmapWidget::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    if (enabled) m_dirty = true;
    update();
}

QSize HeatmapWidget::sizeHint() const
{
    if (m_data.isEmpty()) return QSize(200, 200);
    return QSize(m_data[0].size() * m_cellSize, m_data.size() * m_cellSize);
}

QSize HeatmapWidget::minimumSizeHint() const { return QSize(100, 100); }

void HeatmapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (m_dirty) updatePixmap();
    QPainter p(this);
    p.drawPixmap(0, 0, m_cache);
    if (m_hoverRow >= 0 && m_hoverRow < m_data.size()) {
        auto &row = m_data[m_hoverRow];
        if (m_hoverCol >= 0 && m_hoverCol < row.size()) {
            p.setPen(QPen(Qt::white, 2));
            p.drawRect(m_hoverCol * m_cellSize, m_hoverRow * m_cellSize, m_cellSize, m_cellSize);
        }
    }
}

void HeatmapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_dirty = true;
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int col = event->pos().x() / m_cellSize;
    int row = event->pos().y() / m_cellSize;
    if (row != m_hoverRow || col != m_hoverCol) {
        m_hoverRow = row;
        m_hoverCol = col;
        if (row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size()) {
            emit cellHovered(row, col, m_data[row][col]);
            QToolTip::showText(event->globalPosition().toPoint(),
                formatValue(m_data[row][col]));
        }
        update();
    }
}

void HeatmapWidget::mousePressEvent(QMouseEvent *event)
{
    int col = event->pos().x() / m_cellSize;
    int row = event->pos().y() / m_cellSize;
    if (row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size()) {
        emit cellClicked(row, col, m_data[row][col]);
    }
}

void HeatmapWidget::updatePixmap()
{
    m_cache = QPixmap(size());
    m_cache.fill(Qt::transparent);
    QPainter p(&m_cache);
    p.setRenderHint(QPainter::Antialiasing, false);
    double range = m_maxValue - m_minValue;
    if (range <= 0) range = 1.0;
    for (int r = 0; r < m_data.size(); ++r) {
        for (int c = 0; c < m_data[r].size(); ++c) {
            QColor color = valueToColor(m_data[r][c]);
            p.fillRect(c * m_cellSize, r * m_cellSize, m_cellSize, m_cellSize, color);
            if (m_showValues && m_cellSize >= 20) {
                p.setPen(color.lightnessF() > 0.5 ? Qt::black : Qt::white);
                p.setFont(font());
                p.drawText(QRect(c * m_cellSize, r * m_cellSize, m_cellSize, m_cellSize),
                    Qt::AlignCenter, formatValue(m_data[r][c]));
            }
        }
    }
    m_dirty = false;
}

QColor HeatmapWidget::valueToColor(double value) const
{
    double t = (value - m_minValue) / (m_maxValue - m_minValue);
    t = qBound(0.0, t, 1.0);
    int r = static_cast<int>(t < 0.5 ? 0 : (t - 0.5) * 2 * 255);
    int g = static_cast<int>(t < 0.5 ? t * 2 * 255 : (1.0 - t) * 2 * 255);
    int b = static_cast<int>(t < 0.5 ? (0.5 - t) * 2 * 255 : 0);
    return QColor(r, g, b);
}

QString HeatmapWidget::formatValue(double value) const
{
    if (qAbs(value) < 0.01) return "0";
    if (qAbs(value) >= 1000) return QString::number(value, 'f', 0);
    return QString::number(value, 'f', 2);
}