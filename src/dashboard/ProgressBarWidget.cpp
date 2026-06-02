/**
 * @file ProgressBarWidget.cpp
 * @brief 进度条仪表盘控件实现
 */

#include "dashboard/ProgressBarWidget.h"

/**
 * @brief 构造函数
 * @param parent 父控件
 */
ProgressBarWidget::ProgressBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ProgressBarWidget");
}

/**
 * @brief 设置当前显示值
 * @param value 新值
 */
void ProgressBarWidget::setValue(double value)
{
    m_value = value;
}

/**
 * @brief 设置量程范围
 * @param min 最小值
 * @param max 最大值
 */
void ProgressBarWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
}

/**
 * @brief 设置标签文本
 * @param label 标签
 */
void ProgressBarWidget::setLabel(const QString &label)
{
    m_label = label;
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void ProgressBarWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 绘制事件（暂未实现）
 * @param event 绘制事件参数
 */
void ProgressBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QWidget::paintEvent(event);
}
