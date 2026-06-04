/**
 * @file HeatmapWidget.cpp
 * @brief 热力图控件的实现文件，提供二维数据的颜色可视化渲染与交互功能
 */

#include "chart/heatmap/HeatmapWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>
#include <cmath>

/** @brief 构造函数，初始化热力图控件并设置鼠标追踪和最小尺寸 @param parent 父控件指针 */
HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HeatmapWidget");
    setMouseTracking(true);
    setMinimumSize(100, 100);
}

/** @brief 析构函数，使用默认实现 */
HeatmapWidget::~HeatmapWidget() = default;

/** @brief 设置热力图的二维数据矩阵，自动模式下会重新计算值域范围 @param data 二维浮点数矩阵，外层为行(纵轴)，内层为列(横轴) */
void HeatmapWidget::setData(const QVector<QVector<double>> &data)
{
    m_data = data;
    m_dirty = true;
    m_totalDataUpdates++;
    if (m_autoScale) { m_totalAutoScales++;
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

/** @brief 手动设置颜色映射的值域范围，同时关闭自动缩放 @param min 值域下界 @param max 值域上界 */
void HeatmapWidget::setColorRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    m_totalColorMapChanges++;
    ++m_totalColorScales;
    m_autoScale = false;
    m_dirty = true;
    update();
}

/** @brief 设置每个单元格的像素边长，最小值为4像素 @param size 单元格边长(像素) */
void HeatmapWidget::setCellSize(int size)
{
    m_cellSize = qMax(4, size);
    m_dirty = true;
    update();
}

/** @brief 设置是否在单元格内显示数值文字(单元格>=20px时生效) @param show true=显示数值 */
void HeatmapWidget::setShowValues(bool show)
{
    m_showValues = show;
    m_dirty = true;
    update();
}

/** @brief 设置是否启用自动缩放，启用后setData会自动重新计算值域 @param enabled true=启用自动缩放 */
void HeatmapWidget::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    if (enabled) m_dirty = true;
    update();
}

/** @brief 返回控件的推荐尺寸，基于数据行列数和单元格大小计算 @return 推荐的控件尺寸，空数据时返回200x200 */
QSize HeatmapWidget::sizeHint() const
{
    if (m_data.isEmpty() || m_data[0].isEmpty()) return QSize(200, 200);
    return QSize(m_data[0].size() * m_cellSize, m_data.size() * m_cellSize);
}

/** @brief 返回控件的最小尺寸提示 @return 固定返回100x100像素 */
QSize HeatmapWidget::minimumSizeHint() const { return QSize(100, 100); }

// paintEvent/updatePixmap/valueToColor/formatValue/resetHeatmapStats已移至 HeatmapWidgetPaint.cpp

/** @brief 窗口大小变更事件处理，标记缓存为脏以触发重绘 @param event 大小变更事件参数 */
void HeatmapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_dirty = true;
}

/** @brief 鼠标移动事件处理，计算悬停单元格并发出cellHovered信号和工具提示 @param event 鼠标事件参数 */
void HeatmapWidget::mouseMoveEvent(QMouseEvent *event)
{
    int px = event->pos().x(), py = event->pos().y();
    int col = (px >= 0) ? px / m_cellSize : -1;
    int row = (py >= 0) ? py / m_cellSize : -1;
    if (row != m_hoverRow || col != m_hoverCol) {
        m_hoverRow = row;
        m_hoverCol = col;
        if (row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size()) {
            m_totalCellHovers++;
            emit cellHovered(row, col, m_data[row][col]);
            QToolTip::showText(event->globalPosition().toPoint(),
                formatValue(m_data[row][col]));
        }
        update();
    }
}

/** @brief 鼠标按下事件处理，计算点击的单元格并发出cellClicked信号 @param event 鼠标事件参数 */
void HeatmapWidget::mousePressEvent(QMouseEvent *event)
{
    int px = event->pos().x(), py = event->pos().y();
    int col = (px >= 0) ? px / m_cellSize : -1;
    int row = (py >= 0) ? py / m_cellSize : -1;
    if (row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size()) {
        m_totalCellClicks++;
        emit cellClicked(row, col, m_data[row][col]);
    }
}

/** @brief 滚轮事件处理，通过滚轮调整单元格大小实现缩放 @param event 滚轮事件参数 */
void HeatmapWidget::wheelEvent(QWheelEvent *event)
{
    ++m_totalZoomEvents;
    int delta = event->angleDelta().y() > 0 ? 2 : -2;
    setCellSize(m_cellSize + delta);
    event->accept();
}

