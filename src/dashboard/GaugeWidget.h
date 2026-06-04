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
    /** @brief 构造量表控件 @param parent 父控件指针 */
    explicit GaugeWidget(QWidget *parent = nullptr);

    /** @brief 设置当前显示值 @param value 新的显示值 */
    void setValue(double value);

    /** @brief 设置量程范围 @param min 最小值 @param max 最大值 */
    void setRange(double min, double max);

    /** @brief 设置显示标签文本 @param label 标签字符串 */
    void setLabel(const QString &label);

    /** @brief 绑定数据通道名称 @param channelName 通道名称 */
    void bindChannel(const QString &channelName);

    /** @brief 获取当前显示值 @return 当前值 */
    double value() const { return m_value; }

    /** @brief 获取量程最小值 @return 最小值 */
    double min() const { return m_min; }

    /** @brief 获取量程最大值 @return 最大值 */
    double max() const { return m_max; }

    /** @brief 获取显示标签 @return 标签文本 */
    QString label() const { return m_label; }

    /** @brief 获取绑定的数据通道名 @return 通道名称 */
    QString channelName() const { return m_channelName; }

    // ---- 统计计数接口 ----
    /** @brief 获取累计值更新次数 */
    quint64 totalValueUpdates() const;
    /** @brief 获取累计量程变更次数 */
    quint64 totalRangeChanges() const;
    /** @brief 获取累计阈值超限次数(值超出量程范围) */
    quint64 totalThresholdExceeds() const { return m_totalThresholdExceeds; }
    /** @brief 获取累计渲染次数(paintEvent触发) */
    quint64 totalRenders() const { return m_totalRenders; }
    /** @brief 重置所有量表统计计数器 */
    void resetGaugeStatistics();

protected:
    /** @brief 绘制事件 — 绘制圆形仪表盘、刻度线、指针与数值文本 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 120x120像素 */
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
    quint64 m_totalThresholdExceeds = 0; ///< 累计阈值超限次数(值超出量程范围)
    quint64 m_totalRenders = 0;       ///< 累计渲染次数(paintEvent触发)
};

#endif // GAUGE_WIDGET_H
