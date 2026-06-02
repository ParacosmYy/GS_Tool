/**
 * @file LedIndicatorWidget.h
 * @brief LED 指示灯控件
 *
 * 以圆形指示灯形式展示开关量状态，支持自定义颜色。
 */

#ifndef LED_INDICATOR_WIDGET_H
#define LED_INDICATOR_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QColor>

/**
 * @class LedIndicatorWidget
 * @brief LED 指示灯控件，用于显示开关量状态
 */
class LedIndicatorWidget : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit LedIndicatorWidget(QWidget *parent = nullptr);

    /// 设置开关状态
    void setOn(bool on);

    /// 设置 LED 颜色
    void setColor(const QColor &color);

    /// 绑定数据通道
    void bindChannel(const QString &channelName);

protected:
    /// 绘制事件 —— 暂未实现
    void paintEvent(QPaintEvent *event) override;

private:
    bool    m_on    = false;        ///< 开关状态
    QColor  m_color;                ///< LED 颜色
    QString m_channelName;          ///< 绑定的数据通道名称
};

#endif // LED_INDICATOR_WIDGET_H
