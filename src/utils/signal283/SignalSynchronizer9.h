/**
 * @file SignalSynchronizer9.h
 * @brief 信号同步器(相位相关与sinc插值子采样对齐的精密多通道同步) — Signal Synchronizer with Phase Correlation and Sub-sample Alignment via Sinc Interpolation for Precision Multi-channel Sync
 *
 * 功能: 实现信号同步器(signal synchronizer)，采用相位相关(phase correlation)
 *       与sinc插值子采样对齐(sub-sample alignment via sinc interpolation)实现精密多通道同步(precision multi-channel sync)。
 *
 * 协作: CrossCorrelation8(互相关) / Resampler7(重采样) / FIRFilter6(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号同步器(相位相关与sinc插值子采样对齐)
 */
class SignalSynchronizer9 : public QObject {
    Q_OBJECT

public:
    /** @brief Sync result for a channel pair */
    struct SyncResult {
        double sampleOffset = 0.0;      // Integer + fractional offset
        int integerOffset = 0;
        double fractionalOffset = 0.0;
        double peakCorrelation = 0.0;
        bool valid = false;
    };

    /** @brief Multi-channel alignment result */
    struct MultiSyncResult {
        QVector<double> alignedRef;      // Reference channel (unchanged)
        QVector<QVector<double>> alignedChannels;
        QVector<SyncResult> syncResults;
        double overallQuality = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numChannels = 0;
        int signalLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer9(QObject *parent = nullptr);
    ~SignalSynchronizer9() override;

    /** @brief Set sinc interpolation kernel radius */
    void setSincRadius(int radius);

    /** @brief Set max search range for correlation peak */
    void setMaxSearchRange(int range);

    /** @brief Synchronize single channel to reference */
    SyncResult synchronizePair(const QVector<double>& reference,
                                const QVector<double>& signal);

    /** @brief Synchronize multiple channels to reference */
    MultiSyncResult synchronizeMulti(const QVector<double>& reference,
                                      const QVector<QVector<double>>& channels);

    /** @brief Apply sub-sample shift via sinc interpolation */
    QVector<double> sincInterpolate(const QVector<double>& signal, double offset) const;

    /** @brief Compute phase correlation between two signals */
    QVector<double> phaseCorrelation(const QVector<double>& a,
                                      const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void syncDone(int channel, double offset, double quality, double timeMs);
    void multiSyncDone(int channels, double overallQuality, double timeMs);

private:
    int m_sincRadius = 16;
    int m_maxRange = 1024;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sinc function */
    double sinc(double x) const;

    /** @brief Lanczos window */
    double lanczosWindow(double x) const;

    /** @brief Compute cross-correlation (time domain) */
    QVector<double> crossCorrelation(const QVector<double>& a,
                                      const QVector<double>& b) const;

    /** @brief Find fractional peak via parabolic interpolation */
    double fractionalPeak(const QVector<double>& corr, int intPeak) const;
};
