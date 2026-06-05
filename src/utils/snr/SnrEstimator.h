/**
 * @file SnrEstimator.h
 * @brief 信噪比估计器 — 信号质量评估
 */
#ifndef SNRESTIMATOR_H
#define SNRESTIMATOR_H

#include <QObject>
#include <QVector>

class SnrEstimator : public QObject {
    Q_OBJECT
public:
    struct Result {
        double snrDb = 0.0;
        double signalPower = 0.0;
        double noisePower = 0.0;
        double snrLinear = 0.0;
    };

    struct Stats {
        quint64 totalEstimations = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit SnrEstimator(QObject* parent = nullptr);

    /** @brief 基于已知信号的SNR @param signal 纯信号 @param noisy 含噪信号 @return 结果 */
    Result estimateKnown(const QVector<double>& signal,
                         const QVector<double>& noisy);

    /** @brief 基于信号段的SNR(信号段/噪声段) @param signalSegment 信号段 @param noiseSegment 噪声段 @return 结果 */
    Result estimateSegmented(const QVector<double>& signalSegment,
                              const QVector<double>& noiseSegment);

    /** @brief 基于最小统计量的SNR @param data 含噪信号 @param noiseEstLen 噪声估计长度 @return 结果 */
    Result estimateMinStat(const QVector<double>& data, int noiseEstLen = 100);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimationCompleted(double snrDb);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // SNRESTIMATOR_H
