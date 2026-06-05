/**
 * @file LombPeriodogram.h
 * @brief Lomb-Scargle周期图 — 非均匀采样数据频谱分析
 *
 * 功能: 对非均匀时间采样的数据进行Lomb-Scargle周期图分析，
 *       计算频域功率谱，自动生成频率网格，估计假警概率，
 *       适用于天文观测、不规则采样信号分析。
 *
 * 协作: SpectrumAnalyzer(均匀FFT频谱) / FftEngine(FFT引擎)
 */
#ifndef LOMBPERIODOGRAM_H
#define LOMBPERIODOGRAM_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Lomb-Scargle周期图 — 非均匀采样频谱分析
 */
class LombPeriodogram : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;    ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit LombPeriodogram(QObject* parent = nullptr);

    /** @brief 计算Lomb-Scargle周期图
     *  @param times       采样时间序列
     *  @param values      采样值序列
     *  @param frequencies 频率网格
     *  @return (频率数组, 功率数组) */
    QPair<QVector<double>, QVector<double>> compute(
        const QVector<double>& times,
        const QVector<double>& values,
        const QVector<double>& frequencies);

    /** @brief 自动生成频率网格
     *  @param n       采样点数
     *  @param minFreq 最小频率
     *  @param maxFreq 最大频率
     *  @return 频率网格 */
    QVector<double> autoFrequencies(int n, double minFreq, double maxFreq) const;

    /** @brief 计算假警概率(FAP)
     *  @param power 观测功率
     *  @param n     采样点数
     *  @return 假警概率(0-1) */
    double falseAlarmProbability(double power, int n) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param frequencyCount 频率数 */
    void computationCompleted(int frequencyCount);

private:
    double m_timeSum;              ///< 处理时间累加器

    mutable Stats m_stats;         ///< 可变统计
};

#endif // LOMBPERIODOGRAM_H
