/**
 * @file HeatmapWidgetPaint.cpp
 * @brief 热力图控件 — 绘制与渲染方法实现
 *
 * 从 HeatmapWidget.cpp 拆分而来，包含 paintEvent、
 * updatePixmap、valueToColor、formatValue 等渲染相关方法。
 */

#include "chart/heatmap/HeatmapWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <cmath>

/** @brief 绘制事件处理，将缓存像素图绘制到控件上并叠加悬停高亮框 @param event 绘制事件参数(未使用) */
void HeatmapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    m_totalUpdates++;
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

/** @brief 更新离屏缓存像素图，遍历所有数据单元格进行颜色填充和可选文字绘制 */
void HeatmapWidget::updatePixmap()
{
    m_cache = QPixmap(size());
    m_cache.fill(Qt::transparent);
    QPainter p(&m_cache);
    p.setRenderHint(QPainter::Antialiasing, false);
    double range = m_maxValue - m_minValue;
    if (range <= 0) range = 1.0;
    quint64 cellsThisUpdate = 0;
    for (int r = 0; r < m_data.size(); ++r) {
        for (int c = 0; c < m_data[r].size(); ++c) {
            QColor color = valueToColor(m_data[r][c]);
            p.fillRect(c * m_cellSize, r * m_cellSize, m_cellSize, m_cellSize, color);
            ++cellsThisUpdate;
            if (m_showValues && m_cellSize >= 20) {
                p.setPen(color.lightnessF() > 0.5 ? Qt::black : Qt::white);
                p.setFont(font());
                p.drawText(QRect(c * m_cellSize, r * m_cellSize, m_cellSize, m_cellSize),
                    Qt::AlignCenter, formatValue(m_data[r][c]));
            }
        }
    }
    m_totalCellsRendered += cellsThisUpdate;
    if (cellsThisUpdate > m_peakCellsPerUpdate) m_peakCellsPerUpdate = cellsThisUpdate;
    m_dirty = false;
}

/** @brief 将数值映射为颜色，使用蓝->绿->红的三段线性渐变 @param value 待映射的数值 @return 对应的QColor颜色 */
QColor HeatmapWidget::valueToColor(double value) const
{
    double t = (value - m_minValue) / (m_maxValue - m_minValue);
    t = qBound(0.0, t, 1.0);
    int r = static_cast<int>(t < 0.5 ? 0 : (t - 0.5) * 2 * 255);
    int g = static_cast<int>(t < 0.5 ? t * 2 * 255 : (1.0 - t) * 2 * 255);
    int b = static_cast<int>(t < 0.5 ? (0.5 - t) * 2 * 255 : 0);
    return QColor(r, g, b);
}

/** @brief 格式化数值为显示字符串，根据量级选择精度 @param value 待格式化的数值 @return 格式化后的字符串 */
QString HeatmapWidget::formatValue(double value) const
{
    if (qAbs(value) < 0.01) return "0";
    if (qAbs(value) >= 1000) return QString::number(value, 'f', 0);
    return QString::number(value, 'f', 2);
}

/** @brief 重置所有统计计数器 */
void HeatmapWidget::resetHeatmapStats()
{
    m_totalDataUpdates = 0;
    m_totalCellClicks = 0;
    m_totalCellHovers = 0;
    m_totalAutoScales = 0;
    m_totalUpdates = 0;
    m_totalColorMapChanges = 0;
    m_totalCellsRendered = 0;
    m_peakCellsPerUpdate = 0;
    m_totalColorScales = 0;
    m_totalZoomEvents = 0;
}
