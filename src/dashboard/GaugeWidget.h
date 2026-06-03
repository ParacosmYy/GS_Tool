/**
 * @file GaugeWidget.h
 * @brief 仪表盘量表控件
 *
 * 以圆形仪表盘形式展示单个数值，支持自定义范围与标签。
 */

#ifndef GAUGE_WIDGET_H
#define GAUGE_WIDGET_H

#include <QWidget>
#include <QPaintEvent>

/**
 * @class GaugeWidget
 * @brief 圆形量表控件，用于显示单个模拟量
 */
class GaugeWidget : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit GaugeWidget(QWidget *parent = nullptr);

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

    /// 获取量程最小值
    double min() const { return m_min; }

    /// 获取量程最大值
    double max() const { return m_max; }

    /// 获取标签
    QString label() const { return m_label; }

    /// 获取通道名
    QString channelName() const { return m_channelName; }

    // ---- 统计计数接口 ----
    /** @brief 获取累计值更新次数 */
    quint64 totalValueUpdates() const;
    /** @brief 获取累计量程变更次数 */
    quint64 totalRangeChanges() const;
    /** @brief 重置所有量表统计计数器 */
    void resetGaugeStatistics();

protected:
    /// 绘制事件 —— 绘制圆形仪表盘、刻度与指针
    void paintEvent(QPaintEvent *event) override;

    /// 建议的最小尺寸
    QSize minimumSizeHint() const override;

private:
    double  m_value       = 0.0;      ///< 当前值
    double  m_min         = 0.0;      ///< 最小值
    double  m_max         = 100.0;    ///< 最大值
    QString m_label;                  ///< 标签文本
    QString m_channelName;            ///< 绑定的数据通道名称

    // ---- 统计计数器 ----
    quint64 m_totalValueUpdates = 0;  ///< 累计值更新次数
    quint64 m_totalRangeChanges = 0;  ///< 累计量程变更次数
};

#endif // GAUGE_WIDGET_H
