/**
 * @file NumericDisplayWidget.cpp
 * @brief 数值显示控件实现
 *
 * 自定义 paintEvent 绘制大号数值文本、单位后缀和标签。
 * 所有颜色通过 QPen/QBrush 设置。
 */

#include "dashboard/NumericDisplayWidget.h"

#include <QPainter>

#include "core/theme/ThemeManager.h"

/**
 * @brief 构造函数
 * @param parent 父控件
 */
NumericDisplayWidget::NumericDisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("NumericDisplayWidget");
}

/**
 * @brief 设置当前显示值
 * @param value 新值
 */
void NumericDisplayWidget::setValue(double value)
{
    m_value = value;
    ++m_stats.totalValueUpdates;
    if (value > m_stats.peakValue) m_stats.peakValue = value;
    if (!m_stats.hasValue || value < m_stats.minValue) {
        m_stats.minValue = value;
        m_stats.hasValue = true;
    }
    update();
}

/**
 * @brief 设置显示单位
 * @param unit 单位文本（如 "V"、"mA"）
 */
void NumericDisplayWidget::setUnit(const QString &unit)
{
    if (m_unit != unit) {
        ++m_stats.totalUnitChanges;
    }
    m_unit = unit;
    update();
}

/**
 * @brief 设置小数位数
 * @param precision 小数位数
 */
void NumericDisplayWidget::setPrecision(int precision)
{
    m_precision = precision;
    ++m_stats.totalFormatChanges;
    update();
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void NumericDisplayWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 建议最小尺寸
 * @return 最小尺寸 120×60
 */
QSize NumericDisplayWidget::minimumSizeHint() const
{
    return QSize(120, 60);
}

// paintEvent/resetStatistics已移至 NumericDisplayWidgetPaint.cpp
