/**
 * @file StatisticalMoments.h
 * @brief 统计矩计算器 — 偏度/峰度/高阶矩
 *
 * 功能: 计算均值/方差/偏度/峰度/累积量，支持滑动窗口，
 *       统计计算次数/样本数/耗时。
 */
#ifndef STATISTICALMOMENTS_H
#define STATISTICALMOMENTS_H

#include <QObject>
#include <QVector>

class StatisticalMoments : public QObject {
    Q_OBJECT
public:
    struct Moments {
        double mean = 0.0;
        double variance = 0.0;
        double stdDev = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
        double excessKurtosis = 0.0;
    };

    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalSamplesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit StatisticalMoments(QObject* parent = nullptr);

    /** @brief 计算统计矩 @param data 数据 @return 矩 */
    Moments compute(const QVector<double>& data);

    /** @brief 滑动窗口矩 @param data 数据 @param windowSize 窗口大小 @return 逐窗口矩 */
    QVector<Moments> slidingWindow(const QVector<double>& data, int windowSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // STATISTICALMOMENTS_H
