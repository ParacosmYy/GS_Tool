/**
 * @file GaugeWidget.cpp
 * @brief 仪表盘量表控件实现
 */

#include "dashboard/GaugeWidget.h"

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
}

/**
 * @brief 设置标签文本
 * @param label 标签
 */
void GaugeWidget::setLabel(const QString &label)
{
    m_label = label;
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
 * @brief 绘制事件（暂未实现）
 * @param event 绘制事件参数
 */
void GaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QWidget::paintEvent(event);
}
