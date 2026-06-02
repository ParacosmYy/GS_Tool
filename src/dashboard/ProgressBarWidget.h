/**
 * @file ProgressBarWidget.h
 * @brief 进度条仪表盘控件
 *
 * 以线性进度条形式展示单个数值，支持自定义范围与标签。
 */

#ifndef PROGRESS_BAR_WIDGET_H
#define PROGRESS_BAR_WIDGET_H

#include <QWidget>
#include <QPaintEvent>

/**
 * @class ProgressBarWidget
 * @brief 线性进度条控件，用于显示单个模拟量
 */
class ProgressBarWidget : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit ProgressBarWidget(QWidget *parent = nullptr);

    /// 设置当前值
    void setValue(double value);

    /// 设置量程范围
    void setRange(double min, double max);

    /// 设置显示标签
    void setLabel(const QString &label);

    /// 绑定数据通道
    void bindChannel(const QString &channelName);

    /// 获取当前值
    double value() const { return m_value; }

    /// 获取标签
    QString label() const { return m_label; }

    /// 获取通道名
    QString channelName() const { return m_channelName; }

protected:
    /// 绘制事件 —— 绘制圆角进度条与数值文本
    void paintEvent(QPaintEvent *event) override;

    /// 建议最小尺寸
    QSize minimumSizeHint() const override;

private:
    double  m_value = 0.0;    ///< 当前值
    double  m_min   = 0.0;    ///< 最小值
    double  m_max   = 100.0;  ///< 最大值
    QString m_label;          ///< 标签文本
    QString m_channelName;    ///< 绑定的数据通道名称
};

#endif // PROGRESS_BAR_WIDGET_H
