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
#include <QtGlobal>

/**
 * @class ProgressBarWidget
 * @brief 线性进度条控件，用于显示单个模拟量
 */
class ProgressBarWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造进度条控件 @param parent 父控件指针 */
    explicit ProgressBarWidget(QWidget *parent = nullptr);

    /** @brief 设置当前显示值 @param value 新的显示值 */
    void setValue(double value);

    /** @brief 设置量程范围 @param min 最小值 @param max 最大值 */
    void setRange(double min, double max);

    /** @brief 设置显示标签文本 @param label 标签字符串 */
    void setLabel(const QString &label);

    /** @brief 取消当前进度并重置为起始值 */
    void cancel();

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

    /** @brief 获取值更新总次数 */
    quint64 totalValueUpdates() const { return m_totalValueUpdates; }

    /** @brief 获取范围变更总次数 */
    quint64 totalRangeChanges() const { return m_totalRangeChanges; }

    /** @brief 获取累计完成事件次数(值达到上限) */
    quint64 totalCompleteEvents() const { return m_totalCompleteEvents; }

    /** @brief 获取累计取消事件次数 */
    quint64 totalCancelledEvents() const { return m_totalCancelledEvents; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /** @brief 绘制事件 — 绘制圆角进度条、填充区域、数值文本与标签 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 150x50像素 */
    QSize minimumSizeHint() const override;

private:
    double  m_value = 0.0;    ///< 当前值
    double  m_min   = 0.0;    ///< 最小值
    double  m_max   = 100.0;  ///< 最大值
    QString m_label;          ///< 标签文本
    QString m_channelName;    ///< 绑定的数据通道名称

    quint64 m_totalValueUpdates = 0; ///< 值更新总次数
    quint64 m_totalRangeChanges = 0; ///< 范围变更总次数
    quint64 m_totalCompleteEvents = 0; ///< 累计完成事件次数(值达到上限)
    quint64 m_totalCancelledEvents = 0; ///< 累计取消事件次数
};

#endif // PROGRESS_BAR_WIDGET_H
