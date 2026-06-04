/**
 * @file MiniChartWidget.h
 * @brief 仪表盘迷你折线图控件
 *
 * 以滚动折线图形式展示最近 N 个数据点的实时趋势，
 * 适用于仪表盘中嵌入的小型数据可视化场景。
 * 支持自定义标签、单位、Y轴范围自动适配、平滑曲线绘制。
 */

#ifndef MINI_CHART_WIDGET_H
#define MINI_CHART_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QVector>
#include <QtGlobal>

/**
 * @class MiniChartWidget
 * @brief 迷你折线图控件，用于在仪表盘中显示数据趋势
 * @details 维护一个固定长度的环形缓冲区（默认100个点），
 *          setValue() 追加新数据并自动滚动，paintEvent 绘制
 *          平滑曲线、网格线、当前值和标签。
 */
class MiniChartWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造迷你折线图控件 @param parent 父控件指针 */
    explicit MiniChartWidget(QWidget *parent = nullptr);

    /** @brief 追加一个新数据点并触发重绘 @param value 新的数据值 */
    void setValue(double value);

    /** @brief 设置显示标签文本 @param label 标签字符串 */
    void setLabel(const QString &label);

    /** @brief 绑定数据通道名称 @param name 通道名称 */
    void bindChannel(const QString &name);

    /** @brief 设置显示单位 @param unit 单位字符串，如"V"、"mA" */
    void setUnit(const QString &unit);

    /** @brief 设置Y轴固定范围（autoRange=false时生效） @param min 最小值 @param max 最大值 */
    void setRange(double min, double max);

    /** @brief 设置是否自动适配Y轴范围 @param autoRange true=自动 false=使用固定范围 */
    void setAutoRange(bool autoRange);

    /** @brief 获取最新数据值 @return 最新值 */
    double value() const { return m_value; }

    /** @brief 获取显示标签 @return 标签文本 */
    QString label() const { return m_label; }

    /** @brief 获取绑定的数据通道名 @return 通道名称 */
    QString channelName() const { return m_channelName; }

    /** @brief 获取显示单位 @return 单位字符串 */
    QString unit() const { return m_unit; }

    /** @brief 获取Y轴最小值 @return 最小值 */
    double min() const { return m_min; }

    /** @brief 获取Y轴最大值 @return 最大值 */
    double max() const { return m_max; }

    /** @brief 获取是否自动范围模式 @return true=自动范围 */
    bool autoRange() const { return m_autoRange; }

    // ---- 统计计数接口 ----

    /** @brief 迷你折线图运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalValueUpdates = 0;    ///< 累计值更新次数
        quint64 totalRenders = 0;         ///< 累计渲染次数(paintEvent触发)
        double  peakValue = 0.0;          ///< 历史峰值
        double  minValue = 0.0;           ///< 历史最小值
        bool    hasValue = false;          ///< 是否已有值(用于首次最小值初始化)
        quint64 totalRangeChanges = 0;    ///< 累计范围变更次数
        quint64 totalOverflows = 0;       ///< 累计缓冲区溢出次数(旧数据被覆盖)
    };

    /** @brief 获取累计值更新次数 */
    quint64 totalValueUpdates() const { return m_stats.totalValueUpdates; }
    /** @brief 获取累计渲染次数(paintEvent触发) */
    quint64 totalRenders() const { return m_stats.totalRenders; }
    /** @brief 获取历史峰值 */
    double peakValue() const { return m_stats.peakValue; }
    /** @brief 获取历史最小值 */
    double minValue() const { return m_stats.minValue; }
    /** @brief 获取累计范围变更次数 */
    quint64 totalRangeChanges() const { return m_stats.totalRangeChanges; }
    /** @brief 获取累计缓冲区溢出次数(旧数据被覆盖) */
    quint64 totalOverflows() const { return m_stats.totalOverflows; }
    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /** @brief 绘制事件 — 绘制背景、网格、折线、当前值与标签 @param event 绘制事件参数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 160x80像素 */
    QSize minimumSizeHint() const override;

private:
    /** @brief 根据缓冲区数据计算Y轴可见范围 @return {min, max} 范围对 */
    QPair<double, double> computeVisibleRange() const;

    static constexpr int kMaxPoints = 100; ///< 最大数据点数（环形缓冲区容量）

    double  m_value = 0.0;                 ///< 最新数据值
    double  m_min   = 0.0;                 ///< Y轴最小值（固定范围模式）
    double  m_max   = 100.0;               ///< Y轴最大值（固定范围模式）
    bool    m_autoRange = true;            ///< 是否自动适配Y轴范围
    QString m_label;                       ///< 标签文本
    QString m_channelName;                 ///< 绑定的数据通道名称
    QString m_unit;                        ///< 显示单位

    QVector<double> m_data;                ///< 环形缓冲区，存储历史数据点
    int     m_head = 0;                    ///< 环形缓冲区写入位置
    int     m_count = 0;                   ///< 当前缓冲区中的有效数据点数

    mutable Stats m_stats;                 ///< 聚合统计结构体(mutable因paintEvent为const)
};

#endif // MINI_CHART_WIDGET_H
