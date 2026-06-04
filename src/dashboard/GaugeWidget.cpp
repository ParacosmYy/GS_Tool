/**
 * @file GaugeWidget.cpp
 * @brief 仪表盘量表控件实现
 *
 * 自定义 paintEvent 绘制 270° 弧形刻度盘、大小刻度线、
 * 指针三角形及中心数值文本。所有颜色通过 QPen/QBrush 设置。
 */

#include "dashboard/GaugeWidget.h"

#include <QPainter>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
GaugeWidget::GaugeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("GaugeWidget");
}

/**
 * @brief 设置当前显示值
 * @param value 新值
 */
void GaugeWidget::setValue(double value)
{
    m_value = value;
    ++m_totalValueUpdates;
    if (value < m_min || value > m_max) ++m_totalThresholdExceeds;
    update();
}

/**
 * @brief 设置量程范围
 * @param min 最小值
 * @param max 最大值
 */
void GaugeWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    ++m_totalRangeChanges;
    update();
}

/**
 * @brief 设置标签文本
 * @param label 标签
 */
void GaugeWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void GaugeWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 建议最小尺寸
 * @return 最小尺寸 120×120
 */
QSize GaugeWidget::minimumSizeHint() const
{
    return QSize(120, 120);
}

// paintEvent 已移至 GaugeWidgetPaint.cpp

/** @brief 获取累计值更新次数 */
quint64 GaugeWidget::totalValueUpdates() const
{
    return m_totalValueUpdates;
}

/** @brief 获取累计量程变更次数 */
quint64 GaugeWidget::totalRangeChanges() const
{
    return m_totalRangeChanges;
}

/** @brief 重置所有量表统计计数器 */
void GaugeWidget::resetGaugeStatistics()
{
    m_totalValueUpdates = 0;
    m_totalRangeChanges = 0;
    m_totalThresholdExceeds = 0;
    m_totalRenders = 0;
}
