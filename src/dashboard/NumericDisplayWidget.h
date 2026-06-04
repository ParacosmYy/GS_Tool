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
#include <QtGlobal>

/**
 * @class NumericDisplayWidget
 * @brief 数值显示控件，用于高精度数值展示
 */
class NumericDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造数值显示控件 @param parent 父控件指针 */
    explicit NumericDisplayWidget(QWidget *parent = nullptr);

    /** @brief 设置当前显示值 @param value 新的显示值 */
    void setValue(double value);

    /** @brief 设置显示单位文本 @param unit 单位字符串，如"V"、"mA" */
    void setUnit(const QString &unit);

    /** @brief 设置小数位数 @param precision 小数位数，默认2 */
    void setPrecision(int precision);

    /** @brief 绑定数据通道名称 @param channelName 通道名称 */
    void bindChannel(const QString &channelName);

    /** @brief 获取当前显示值 @return 当前值 */
    double value() const { return m_value; }

    /** @brief 获取显示单位 @return 单位字符串 */
    QString unit() const { return m_unit; }

    /** @brief 获取小数位数 @return 精度位数 */
    int precision() const { return m_precision; }

    /** @brief 获取绑定的数据通道名 @return 通道名称 */
    QString channelName() const { return m_channelName; }

    /** @brief 数值显示运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalValueUpdates = 0;  ///< 值更新总次数
        quint64 totalFormatChanges = 0; ///< 格式变更总次数
        quint64 totalUnitChanges = 0;   ///< 单位变更总次数
        double  peakValue = 0.0;        ///< 历史峰值
        double  minValue = 0.0;         ///< 历史最小值
        bool    hasValue = false;        ///< 是否已有值(用于首次最小值初始化)
    };

    /** @brief 获取值更新总次数 */
    quint64 totalValueUpdates() const { return m_stats.totalValueUpdates; }

    /** @brief 获取格式变更总次数 */
    quint64 totalFormatChanges() const { return m_stats.totalFormatChanges; }

    /** @brief 获取单位变更总次数 */
    quint64 totalUnitChanges() const { return m_stats.totalUnitChanges; }

    /** @brief 获取历史峰值 */
    double peakValue() const { return m_stats.peakValue; }

    /** @brief 获取历史最小值 */
    double minValue() const { return m_stats.minValue; }

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /** @brief 绘制事件 — 绘制背景矩形、通道标签、大号数值和单位后缀 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 120x60像素 */
    QSize minimumSizeHint() const override;

private:
    double  m_value    = 0.0;   ///< 当前值
    QString m_unit;             ///< 显示单位
    int     m_precision = 2;    ///< 小数位数
    QString m_channelName;      ///< 绑定的数据通道名称

    mutable Stats m_stats;    ///< 聚合统计结构体(mutable因paintEvent为const)
};

#endif // NUMERIC_DISPLAY_WIDGET_H
