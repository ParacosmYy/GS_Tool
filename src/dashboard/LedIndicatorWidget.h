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
    /** @brief 构造LED指示灯控件 @param parent 父控件指针 */
    explicit LedIndicatorWidget(QWidget *parent = nullptr);

    /** @brief 设置开关状态 @param on true=亮，false=灭 */
    void setOn(bool on);

    /** @brief 设置LED颜色 @param color 颜色值 */
    void setColor(const QColor &color);

    /** @brief 绑定数据通道名称 @param channelName 通道名称 */
    void bindChannel(const QString &channelName);

    /** @brief 获取开关状态 @return true=亮，false=灭 */
    bool isOn() const { return m_on; }

    /** @brief 获取LED颜色 @return 当前颜色 */
    QColor color() const { return m_color; }

    /** @brief 获取绑定的数据通道名 @return 通道名称 */
    QString channelName() const { return m_channelName; }

    /** @brief LED指示灯运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalStateChanges = 0;       ///< 状态切换总次数
        quint64 totalBlinks = 0;             ///< 闪烁总次数(paintEvent中递增)
        quint64 totalColorChanges = 0;       ///< 颜色变更总次数
        qint64  totalOnDurationMs = 0;       ///< 累计亮灯持续时间(毫秒)
        qint64  totalOffDurationMs = 0;      ///< 累计灭灯持续时间(毫秒)
        qint64  lastStateChangeMs = 0;       ///< 上次状态切换时间戳(毫秒)
    };

    /** @brief 获取状态切换总次数 */
    quint64 totalStateChanges() const { return m_stats.totalStateChanges; }

    /** @brief 获取闪烁总次数 */
    quint64 totalBlinks() const { return m_stats.totalBlinks; }

    /** @brief 获取颜色变更总次数 */
    quint64 totalColorChanges() const { return m_stats.totalColorChanges; }

    /** @brief 获取累计亮灯持续时间(毫秒) @return 持续时间 */
    qint64 totalOnDurationMs() const { return m_stats.totalOnDurationMs; }

    /** @brief 获取累计灭灯持续时间(毫秒) @return 持续时间 */
    qint64 totalOffDurationMs() const { return m_stats.totalOffDurationMs; }

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /** @brief 绘制事件 — 绘制圆形LED指示灯、径向渐变发光效果和高光 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 40x40像素 */
    QSize minimumSizeHint() const override;

private:
    bool    m_on    = false;        ///< 开关状态
    QColor  m_color;                ///< LED 颜色
    QString m_channelName;          ///< 绑定的数据通道名称

    mutable Stats m_stats;    ///< 聚合统计结构体(mutable因paintEvent为const)
};

#endif // LED_INDICATOR_WIDGET_H
