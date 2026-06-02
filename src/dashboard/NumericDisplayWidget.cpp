/**
 * @file NumericDisplayWidget.cpp
 * @brief 数值显示控件实现
 */

#include "dashboard/NumericDisplayWidget.h"

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
}

/**
 * @brief 设置显示单位
 * @param unit 单位文本（如 "V"、"mA"）
 */
void NumericDisplayWidget::setUnit(const QString &unit)
{
    m_unit = unit;
}

/**
 * @brief 设置小数位数
 * @param precision 小数位数
 */
void NumericDisplayWidget::setPrecision(int precision)
{
    m_precision = precision;
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
 * @brief 绘制事件（暂未实现）
 * @param event 绘制事件参数
 */
void NumericDisplayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QWidget::paintEvent(event);
}
