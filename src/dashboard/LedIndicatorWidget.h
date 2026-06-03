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
#include <QtGlobal>

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

    /// 获取开关状态
    bool isOn() const { return m_on; }

    /// 获取颜色
    QColor color() const { return m_color; }

    /// 获取通道名
    QString channelName() const { return m_channelName; }

    /** @brief 获取状态切换总次数 */
    quint64 totalStateChanges() const { return m_totalStateChanges; }

    /** @brief 获取闪烁总次数 */
    quint64 totalBlinks() const { return m_totalBlinks; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /// 绘制事件 —— 绘制圆形 LED
    void paintEvent(QPaintEvent *event) override;

    /// 建议最小尺寸
    QSize minimumSizeHint() const override;

private:
    bool    m_on    = false;        ///< 开关状态
    QColor  m_color;                ///< LED 颜色
    QString m_channelName;          ///< 绑定的数据通道名称

    quint64 m_totalStateChanges = 0;       ///< 状态切换总次数
    mutable quint64 m_totalBlinks = 0;     ///< 闪烁总次数(paintEvent中递增)
};

#endif // LED_INDICATOR_WIDGET_H
