/**
 * @file RaisedCosineFilter.h
 * @brief 升余弦滤波器 — 脉冲成形滤波器设计与滤波
 *
 * 功能: 设计升余弦(根升余弦)脉冲成形滤波器，支持可配置
 *       滚降系数、符号采样数和抽头数，对信号执行FIR滤波。
 *
 * 协作: DigitalFilter(通用滤波) / WaveformGenerator(波形生成)
 */
#ifndef RAISEDCOSINEFILTER_H
#define RAISEDCOSINEFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 升余弦/根升余弦脉冲成形滤波器
 */
class RaisedCosineFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum class FilterType {
        RaisedCosine,       ///< 升余弦
        RootRaisedCosine    ///< 根升余弦
    };
    Q_ENUM(FilterType)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDesigned = 0;          ///< 累计设计次数
        quint64 totalFiltered = 0;          ///< 累计滤波次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit RaisedCosineFilter(QObject* parent = nullptr);

    void setFilterType(FilterType type);

    /**
     * @brief 设计滤波器系数
     * @param taps 抽头数(建议奇数)
     * @param rolloff 滚降系数[0,1]
     * @param samplesPerSymbol 每符号采样数
     * @return 滤波器脉冲响应系数
     */
    QVector<double> design(int taps, double rolloff, double samplesPerSymbol);

    /**
     * @brief 对输入信号执行FIR滤波
     * @param input 输入信号
     * @return 滤波后信号
     */
    QVector<double> filter(const QVector<double>& input);

    /** @brief 获取当前滤波器系数 */
    QVector<double> coefficients() const { return m_coeffs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成信号 @param taps 抽头数 */
    void filterDesigned(int taps);

private:
    FilterType      m_filterType;       ///< 滤波器类型
    QVector<double> m_coeffs;           ///< 当前滤波器系数
    double          m_timeSum;          ///< 累计耗时(ms)
    mutable Stats   m_stats;            ///< 可变统计
};

#endif // RAISEDCOSINEFILTER_H
