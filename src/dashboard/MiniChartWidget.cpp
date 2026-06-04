/**
 * @file MiniChartWidget.cpp
 * @brief 仪表盘迷你折线图控件实现
 *
 * 管理100点环形缓冲区的数据追加与Y轴自动范围计算。
 * paintEvent 绘制逻辑见 MiniChartWidgetPaint.cpp。
 */

#include "dashboard/MiniChartWidget.h"

/**
 * @brief 构造函数
 * @param parent 父控件
 */
MiniChartWidget::MiniChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_data(kMaxPoints, 0.0)
{
    setObjectName("MiniChartWidget");
}

/**
 * @brief 追加一个新数据点并触发重绘
 * @param value 新的数据值
 */
void MiniChartWidget::setValue(double value)
{
    /* 环形缓冲区写入 */
    if (m_count >= kMaxPoints) {
        ++m_stats.totalOverflows;
    }
    m_data[m_head] = value;
    m_head = (m_head + 1) % kMaxPoints;
    if (m_count < kMaxPoints) {
        ++m_count;
    }

    m_value = value;
    ++m_stats.totalValueUpdates;

    /* 统计: 峰值/最小值 */
    if (value > m_stats.peakValue) {
        m_stats.peakValue = value;
    }
    if (!m_stats.hasValue || value < m_stats.minValue) {
        m_stats.minValue = value;
        m_stats.hasValue = true;
    }

    update();
}

/**
 * @brief 设置显示标签文本
 * @param label 标签
 */
void MiniChartWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

/**
 * @brief 绑定数据通道名称
 * @param name 通道名称
 */
void MiniChartWidget::bindChannel(const QString &name)
{
    m_channelName = name;
}

/**
 * @brief 设置显示单位
 * @param unit 单位文本（如 "V"、"mA"）
 */
void MiniChartWidget::setUnit(const QString &unit)
{
    m_unit = unit;
    update();
}

/**
 * @brief 设置Y轴固定范围（autoRange=false时生效）
 * @param min 最小值
 * @param max 最大值
 */
void MiniChartWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    ++m_stats.totalRangeChanges;
    update();
}

/**
 * @brief 设置是否自动适配Y轴范围
 * @param autoRange true=自动 false=使用固定范围
 */
void MiniChartWidget::setAutoRange(bool autoRange)
{
    m_autoRange = autoRange;
    update();
}

/**
 * @brief 建议最小尺寸
 * @return 最小尺寸 160x80
 */
QSize MiniChartWidget::minimumSizeHint() const
{
    return QSize(160, 80);
}

/**
 * @brief 根据缓冲区数据计算Y轴可见范围
 * @return {min, max} 范围对，无数据时返回 {0, 100}
 */
QPair<double, double> MiniChartWidget::computeVisibleRange() const
{
    if (m_count == 0) {
        return {0.0, 100.0};
    }

    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();

    for (int i = 0; i < m_count; ++i) {
        int idx = (m_head - m_count + i + kMaxPoints) % kMaxPoints;
        double v = m_data[idx];
        if (v < lo) lo = v;
        if (v > hi) hi = v;
    }

    /* 若所有值相同，扩展范围避免零高度 */
    if (qFuzzyCompare(lo, hi)) {
        lo -= 1.0;
        hi += 1.0;
    }

    /* 上下各留10%的边距 */
    double range = hi - lo;
    double margin = range * 0.1;
    return {lo - margin, hi + margin};
}

// paintEvent 已移至 MiniChartWidgetPaint.cpp

/**
 * @brief 重置所有统计计数器为零
 */
void MiniChartWidget::resetStatistics()
{
    m_stats = Stats{};
}
