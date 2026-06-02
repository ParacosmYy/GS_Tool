/**
 * @file NumericDisplayWidget.h
 * @brief 数值显示控件
 *
 * 以数字形式展示单个数值，支持单位与精度设置。
 */

#ifndef NUMERIC_DISPLAY_WIDGET_H
#define NUMERIC_DISPLAY_WIDGET_H

#include <QWidget>
#include <QPaintEvent>

/**
 * @class NumericDisplayWidget
 * @brief 数值显示控件，用于高精度数值展示
 */
class NumericDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit NumericDisplayWidget(QWidget *parent = nullptr);

    /// 设置当前值
    void setValue(double value);

    /// 设置显示单位
    void setUnit(const QString &unit);

    /// 设置小数位数
    void setPrecision(int precision);

    /// 绑定数据通道
    void bindChannel(const QString &channelName);

protected:
    /// 绘制事件 —— 暂未实现
    void paintEvent(QPaintEvent *event) override;

private:
    double  m_value    = 0.0;   ///< 当前值
    QString m_unit;             ///< 显示单位
    int     m_precision = 2;    ///< 小数位数
    QString m_channelName;      ///< 绑定的数据通道名称
};

#endif // NUMERIC_DISPLAY_WIDGET_H
