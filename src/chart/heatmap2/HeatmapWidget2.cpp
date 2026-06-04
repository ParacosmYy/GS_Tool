/**
 * @file HeatmapWidget2.cpp
 * @brief 热力图控件变体2 — 带行列标签和网格线的数据矩阵可视化
 */

#include "chart/heatmap2/HeatmapWidget2.h"
#include "core/theme/ThemeManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

/** @brief 构造函数，初始化热力图控件并设置objectName @param parent 父控件指针 */
HeatmapWidget::HeatmapWidget(QWidget *parent) : QWidget(parent) { setObjectName("HeatmapWidget2"); }

/** @brief 析构函数，使用默认实现 */
HeatmapWidget::~HeatmapWidget() = default;

/** @brief 设置二维数据矩阵并触发重绘 @param m 二维浮点数矩阵 */
void HeatmapWidget::setData(const QVector<QVector<double>> &m) { m_data = m; m_totalDataUpdates++; update(); }

/** @brief 设置单元格的宽高像素值 @param w 单元格宽度(像素) @param h 单元格高度(像素) */
void HeatmapWidget::setCellSize(int w, int h) { m_cellW = w; m_cellH = h; update(); }

/** @brief 设置颜色映射的值域范围 @param min 值域下界 @param max 值域上界 */
void HeatmapWidget::setColorRange(double min, double max) { m_minVal = min; m_maxVal = max; update(); }

/** @brief 设置行列标签文本 @param rows 行标签列表 @param cols 列标签列表 */
void HeatmapWidget::setLabels(const QStringList &rows, const QStringList &cols) { m_rowLabels = rows; m_colLabels = cols; update(); }

/** @brief 设置网格线可见性 @param v true=显示网格线 */
void HeatmapWidget::setGridVisible(bool v) { m_gridVisible = v; update(); }

/** @brief 获取数据行数 @return 矩阵行数 */
int HeatmapWidget::rowCount() const { return m_data.size(); }

/** @brief 获取数据列数 @return 矩阵列数，空数据返回0 */
int HeatmapWidget::colCount() const { return m_data.isEmpty() ? 0 : m_data[0].size(); }

/** @brief 获取指定行列位置的数值 @param r 行索引 @param c 列索引 @return 对应位置的数值，越界返回0 */
double HeatmapWidget::valueAt(int r, int c) const { return (r>=0 && r<m_data.size() && c>=0 && c<m_data[r].size()) ? m_data[r][c] : 0; }

/** @brief 绘制事件处理，遍历数据矩阵填充颜色并绘制网格线 @param event 绘制事件参数(未使用) */
void HeatmapWidget::paintEvent(QPaintEvent *) {
    m_totalRepaints++;
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, false);
    if (m_data.isEmpty()) return;
    QColor gridColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Border);
    for (int r = 0; r < m_data.size(); ++r) {
        for (int c = 0; c < m_data[r].size(); ++c) {
            QColor color = valueToColor(m_data[r][c]);
            p.setBrush(color);
            p.setPen(m_gridVisible ? gridColor : Qt::NoPen);
            p.drawRect(c*m_cellW, r*m_cellH, m_cellW, m_cellH);
        }
    }
}

/** @brief 鼠标按下事件处理，计算点击单元格并发出cellClicked信号 @param e 鼠标事件参数 */
void HeatmapWidget::mousePressEvent(QMouseEvent *e) {
    auto cell = posToCell(e->pos());
    if (cell.first >= 0) { m_totalCellClicks++; emit cellClicked(cell.first, cell.second, valueAt(cell.first, cell.second)); }
}

/** @brief 鼠标移动事件处理，计算悬停单元格并发出cellHovered信号 @param e 鼠标事件参数 */
void HeatmapWidget::mouseMoveEvent(QMouseEvent *e) {
    auto cell = posToCell(e->pos());
    if (cell.first >= 0) { m_totalCellHovers++; emit cellHovered(cell.first, cell.second, valueAt(cell.first, cell.second)); }
}

/** @brief 窗口大小变更事件处理 @param e 大小变更事件参数 */
void HeatmapWidget::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }

/** @brief 将数值映射为颜色，蓝→白→红三段线性渐变 @param v 待映射的数值 @return 对应的QColor颜色 */
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

/** @brief 将像素坐标转换为数据矩阵的行列索引 @param pos 像素坐标 @return QPair<行,列>，越界返回<-1,-1> */
QPair<int,int> HeatmapWidget::posToCell(const QPoint &pos) const {
    int c = pos.x() / m_cellW, r = pos.y() / m_cellH;
    if (r >= 0 && r < m_data.size() && c >= 0 && c < m_data[r].size()) return {r,c};
    return {-1,-1};
}

/** @brief 获取数据矩阵总单元格数 @return 所有行的列数之和 */
int HeatmapWidget::totalCells() const {
    int total = 0;
    for (const auto& row : m_data) total += row.size();
    return total;
}

/** @brief 重置所有统计计数器 */
void HeatmapWidget::resetStats() {
    m_totalDataUpdates = 0;
    m_totalCellClicks = 0;
    m_totalCellHovers = 0;
    m_totalRepaints = 0;
}
