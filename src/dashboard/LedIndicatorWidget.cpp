/**
 * @file LedIndicatorWidget.cpp
 * @brief LED 指示灯控件实现
 */

#include "dashboard/LedIndicatorWidget.h"

/**
 * @brief 构造函数
 * @param parent 父控件
 */
LedIndicatorWidget::LedIndicatorWidget(QWidget *parent)
    : QWidget(parent)
    , m_color(Qt::green)
{
    setObjectName("LedIndicatorWidget");
}

/**
 * @brief 设置开关状态
 * @param on true=亮，false=灭
 */
void LedIndicatorWidget::setOn(bool on)
{
    m_on = on;
}

/**
 * @brief 设置 LED 颜色
 * @param color 颜色
 */
void LedIndicatorWidget::setColor(const QColor &color)
{
    m_color = color;
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void LedIndicatorWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 绘制事件（暂未实现）
 * @param event 绘制事件参数
 */
void LedIndicatorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QWidget::paintEvent(event);
}
