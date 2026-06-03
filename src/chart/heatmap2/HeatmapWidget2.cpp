#include "chart/heatmap2/HeatmapWidget2.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

HeatmapWidget::HeatmapWidget(QWidget *parent) : QWidget(parent) { setObjectName("HeatmapWidget2"); }
HeatmapWidget::~HeatmapWidget() = default;

void HeatmapWidget::setData(const QVector<QVector<double>> &m) { m_data = m; update(); }
void HeatmapWidget::setCellSize(int w, int h) { m_cellW = w; m_cellH = h; update(); }
void HeatmapWidget::setColorRange(double min, double max) { m_minVal = min; m_maxVal = max; update(); }
void HeatmapWidget::setLabels(const QStringList &rows, const QStringList &cols) { m_rowLabels = rows; m_colLabels = cols; update(); }
void HeatmapWidget::setGridVisible(bool v) { m_gridVisible = v; update(); }
int HeatmapWidget::rowCount() const { return m_data.size(); }
int HeatmapWidget::colCount() const { return m_data.isEmpty() ? 0 : m_data[0].size(); }
double HeatmapWidget::valueAt(int r, int c) const { return (r>=0 && r<m_data.size() && c>=0 && c<m_data[r].size()) ? m_data[r][c] : 0; }

void HeatmapWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, false);
    if (m_data.isEmpty()) return;
    for (int r = 0; r < m_data.size(); ++r) {
        for (int c = 0; c < m_data[r].size(); ++c) {
            QColor color = valueToColor(m_data[r][c]);
            p.setBrush(color);
            p.setPen(m_gridVisible ? QColor(40,40,40) : Qt::NoPen);
            p.drawRect(c*m_cellW, r*m_cellH, m_cellW, m_cellH);
        }
    }
}

void HeatmapWidget::mousePressEvent(QMouseEvent *e) {
    auto cell = posToCell(e->pos());
    if (cell.first >= 0) emit cellClicked(cell.first, cell.second, valueAt(cell.first, cell.second));
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *e) {
    auto cell = posToCell(e->pos());
    if (cell.first >= 0) emit cellHovered(cell.first, cell.second, valueAt(cell.first, cell.second));
}

void HeatmapWidget::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }

QColor HeatmapWidget::valueToColor(double v) const {
    double t = qBound(0.0, (v - m_minVal) / qMax(m_maxVal - m_minVal, 1e-9), 1.0);
    if (t < 0.5) {
        double s = t * 2;
        return QColor(static_cast<int>(s*255), static_cast<int>(s*255), 255);
    } else {
        double s = (t - 0.5) * 2;
        return QColor(255, static_cast<int>((1-s)*255), static_cast<int>((1-s)*255));
    }
}

QPair<int,int> HeatmapWidget::posToCell(const QPoint &pos) const {
    int c = pos.x() / m_cellW, r = pos.y() / m_cellH;
    if (r >= 0 && r < m_data.size() && c >= 0 && c < m_data[r].size()) return {r,c};
    return {-1,-1};
}
