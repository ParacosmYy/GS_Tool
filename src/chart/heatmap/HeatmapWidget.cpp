/**
 * @file HeatmapWidget.cpp
 * @brief 热力图控件的实现文件，提供二维数据的颜色可视化渲染与交互功能
 */

#include "chart/heatmap/HeatmapWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
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

/**
 * @brief 设置热力图的二维数据矩阵，自动模式下会重新计算值域范围
 * @param data 二维浮点数矩阵，外层为行(纵轴)，内层为列(横轴)
 */
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

/**
 * @brief 手动设置颜色映射的值域范围，同时关闭自动缩放
 * @param min 值域下界
 * @param max 值域上界
 */
void HeatmapWidget::setColorRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    m_autoScale = false;
    m_dirty = true;
    update();
}

/**
 * @brief 设置每个单元格的像素边长，最小值为4像素
 * @param size 单元格边长（像素）
 */
void HeatmapWidget::setCellSize(int size)
{
    m_cellSize = qMax(4, size);
    m_dirty = true;
    update();
}

/**
 * @brief 设置是否在单元格内显示数值文字（单元格>=20px时生效）
 * @param show true表示显示数值，false表示隐藏
 */
void HeatmapWidget::setShowValues(bool show)
{
    m_showValues = show;
    m_dirty = true;
    update();
}

/**
 * @brief 设置是否启用自动缩放，启用后setData会自动重新计算值域
 * @param enabled true表示启用自动缩放
 */
void HeatmapWidget::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    if (enabled) m_dirty = true;
    update();
}

/**
 * @brief 返回控件的推荐尺寸，基于数据行列数和单元格大小计算
 * @return 推荐的控件尺寸，空数据时返回200x200
 */
QSize HeatmapWidget::sizeHint() const
{
    if (m_data.isEmpty()) return QSize(200, 200);
    return QSize(m_data[0].size() * m_cellSize, m_data.size() * m_cellSize);
}

/** @brief 返回控件的最小尺寸提示 @return 固定返回100x100像素 */
QSize HeatmapWidget::minimumSizeHint() const { return QSize(100, 100); }

/**
 * @brief 绘制事件处理，将缓存像素图绘制到控件上并叠加悬停高亮框
 * @param event 绘制事件参数（未使用）
 */
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

/**
 * @brief 窗口大小变更事件处理，标记缓存为脏以触发重绘
 * @param event 大小变更事件参数
 */
void HeatmapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_dirty = true;
}

/**
 * @brief 鼠标移动事件处理，计算悬停单元格并发出cellHovered信号和工具提示
 * @param event 鼠标事件参数，包含鼠标位置信息
 */
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

/**
 * @brief 鼠标按下事件处理，计算点击的单元格并发出cellClicked信号
 * @param event 鼠标事件参数，包含点击位置信息
 */
void HeatmapWidget::mousePressEvent(QMouseEvent *event)
{
    int col = event->pos().x() / m_cellSize;
    int row = event->pos().y() / m_cellSize;
    if (row >= 0 && row < m_data.size() && col >= 0 && col < m_data[row].size()) {
        emit cellClicked(row, col, m_data[row][col]);
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

/**
 * @brief 将数值映射为颜色，使用蓝->绿->红的三段线性渐变
 * @param value 待映射的数值
 * @return 对应的QColor颜色
 */
QColor HeatmapWidget::valueToColor(double value) const
{
    double t = (value - m_minValue) / (m_maxValue - m_minValue);
    t = qBound(0.0, t, 1.0);
    int r = static_cast<int>(t < 0.5 ? 0 : (t - 0.5) * 2 * 255);
    int g = static_cast<int>(t < 0.5 ? t * 2 * 255 : (1.0 - t) * 2 * 255);
    int b = static_cast<int>(t < 0.5 ? (0.5 - t) * 2 * 255 : 0);
    return QColor(r, g, b);
}

/**
 * @brief 格式化数值为显示字符串，根据量级选择精度
 * @param value 待格式化的数值
 * @return 格式化后的字符串，接近0显示"0"，>=1000取整，其余保留2位小数
 */
QString HeatmapWidget::formatValue(double value) const
{
    if (qAbs(value) < 0.01) return "0";
    if (qAbs(value) >= 1000) return QString::number(value, 'f', 0);
    return QString::number(value, 'f', 2);
}