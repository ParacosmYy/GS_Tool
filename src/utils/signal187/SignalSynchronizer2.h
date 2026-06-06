/**
 * @file SignalSynchronizer2.h
 * @brief 信号同步(互相关峰值检测+分数延迟插值) — Signal Synchronization via Cross-Correlation Peak Detection with Fractional Delay Interpolation
 *
 * 功能: 实现信号同步器，支持互相关峰值检测、分数延迟插值、
 *       抛物线精化峰值估计和时延对齐。
 *
 * 协作: CrossCorrelator3(互相关) / FractionalDelay2(分数延迟) / SignalResampler4(重采样)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号同步器(互相关峰值+分数延迟插值)
 */
class SignalSynchronizer2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSyncs = 0;
        int signalLength = 0;
        double detectedDelay = 0.0;
        double peakCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Sync result */
    struct SyncResult {
        double delaySamples = 0.0;
        double peakCorrelation = 0.0;
        int integerDelay = 0;
        double fractionalDelay = 0.0;
        QVector<double> crossCorrelation;
    };

    explicit SignalSynchronizer2(QObject *parent = nullptr);
    ~SignalSynchronizer2() override;

    void setSearchRange(int minDelay, int maxDelay);

    /** @brief 同步参考信号和延迟信号 */
    SyncResult synchronize(const QVector<double>& reference,
                           const QVector<double>& delayed);

    /** @brief 计算互相关 */
    QVector<double> crossCorrelate(const QVector<double>& a,
                                   const QVector<double>& b) const;

    /** @brief 抛物线精化峰值位置 */
    double refinePeak(const QVector<double>& corr, int peakIdx) const;

    /** @brief 应用分数延迟插值对齐 */
    QVector<double> applyDelay(const QVector<double>& signal,
                               double delay) const;

    /** @brief sinc插值 */
    double sincInterpolate(const QVector<double>& signal,
                           double index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void syncCompleted(double delay, double peakCorr);

private:
    int m_minDelay = 0;
    int m_maxDelay = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sinc function */
    double sinc(double x) const;
};
