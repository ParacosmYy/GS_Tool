/**
 * @file SignalSynchronizer8.h
 * @brief 信号同步器(互相关峰值检测与分数延迟插值多通道对齐) — Signal Synchronizer with Cross-Correlation Peak Detection and Fractional Delay Interpolation for Multi-Channel Alignment
 *
 * 功能: 实现信号同步器(Signal synchronizer)，采用互相关峰值检测(cross-correlation peak detection)
 *       和分数延迟插值(fractional delay interpolation)实现多通道对齐(multi-channel alignment)。
 *
 * 协作: SignalFilter6(信号滤波) / SignalResampler7(重采样) / CrossCorrelation5(互相关)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号同步器(互相关峰值检测与分数延迟插值多通道对齐)
 */
class SignalSynchronizer8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numChannels = 0;
        int signalLength = 0;
        double maxDelay = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer8(QObject *parent = nullptr);
    ~SignalSynchronizer8() override;

    /** @brief Set reference signal */
    void setReference(const QVector<double>& ref);

    /** @brief Compute cross-correlation and find peak lag */
    int findPeakLag(const QVector<double>& signal) const;

    /** @brief Compute fractional delay using parabolic interpolation around peak */
    double findFractionalDelay(const QVector<double>& signal) const;

    /** @brief Align signal to reference using detected delay */
    QVector<double> alignSignal(const QVector<double>& signal, double delay) const;

    /** @brief Synchronize multiple channels to reference */
    QVector<QVector<double>> synchronize(
        const QVector<QVector<double>>& channels) const;

    /** @brief Compute normalized cross-correlation */
    QVector<double> crossCorrelation(const QVector<double>& a,
                                      const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void synchronizationCompleted(int numChannels, double maxDelay, double timeMs);

private:
    QVector<double> m_reference;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Lagrange interpolation for fractional delay */
    double lagrangeInterpolate(const QVector<double>& signal,
                                double fractionalIndex) const;

    /** @brief Compute mean of a signal */
    static double mean(const QVector<double>& signal);

    /** @brief Compute standard deviation */
    static double stddev(const QVector<double>& signal);
};
