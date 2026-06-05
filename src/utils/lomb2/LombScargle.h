/**
 * @file LombScargle.h
 * @brief Lomb-Scargle周期图 — 非均匀采样频谱分析
 *
 * 功能: 计算非均匀采样数据的Lomb-Scargle周期图，
 *       自动频率网格，峰值检测，统计计算次数/耗时。
 */
#ifndef LOMBSCARGLE_H
#define LOMBSCARGLE_H

#include <QObject>
#include <QVector>
#include <QPair>

class LombScargle : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit LombScargle(QObject* parent = nullptr);

    /** @brief 计算Lomb-Scargle周期图 @param times 时间点 @param values 对应值 @param freqs 频率网格 @return 功率谱 */
    QVector<double> compute(const QVector<double>& times,
                            const QVector<double>& values,
                            const QVector<double>& freqs);

    /** @brief 自动生成频率网格 @param times 时间点 @param minFreq 最小频率 @param maxFreq 最大频率 @param numFreqs 频率数 @return 频率数组 */
    QVector<double> autoFrequency(const QVector<double>& times,
                                  double minFreq, double maxFreq,
                                  int numFreqs);

    /** @brief 检测峰值 @param power 功率谱 @param freqs 频率网格 @param threshold 阈值(相对最大值) @return 峰值频率和功率 */
    QVector<QPair<double, double>> findPeaks(
        const QVector<double>& power,
        const QVector<double>& freqs,
        double threshold = 0.5);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 周期图计算完成 @param frequencies 频率数 */
    void periodogramComputed(int frequencies);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // LOMBSCARGLE_H
