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

    /// 获取当前值
    double value() const { return m_value; }

    /// 获取单位
    QString unit() const { return m_unit; }

    /// 获取精度
    int precision() const { return m_precision; }

    /// 获取通道名
    QString channelName() const { return m_channelName; }

    /** @brief 获取值更新总次数 */
    quint64 totalValueUpdates() const { return m_totalValueUpdates; }

    /** @brief 获取格式变更总次数 */
    quint64 totalFormatChanges() const { return m_totalFormatChanges; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /// 绘制事件 —— 绘制大号数值、单位与标签
    void paintEvent(QPaintEvent *event) override;

    /// 建议最小尺寸
    QSize minimumSizeHint() const override;

private:
    double  m_value    = 0.0;   ///< 当前值
    QString m_unit;             ///< 显示单位
    int     m_precision = 2;    ///< 小数位数
    QString m_channelName;      ///< 绑定的数据通道名称

    quint64 m_totalValueUpdates = 0;  ///< 值更新总次数
    quint64 m_totalFormatChanges = 0; ///< 格式变更总次数
};

#endif // NUMERIC_DISPLAY_WIDGET_H
