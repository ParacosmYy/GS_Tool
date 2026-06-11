/**
 * @file SignalSynchronizer10.h
 * @brief 信号同步器(互相关峰值插值与亚采样延迟估计实现精密时间对齐) — Signal Synchronizer with Cross-correlation Peak Interpolation and Subsample Delay Estimation for Precision Time Alignment
 *
 * 功能: 实现信号同步器(signal synchronizer)，采用互相关峰值插值(cross-correlation peak interpolation)
 *       与亚采样延迟估计(subsample delay estimation)实现精密时间对齐(precision time alignment)。
 *
 * 协作: CrossCorrelator9(互相关) / Resampler8(重采样) / HilbertTransform7(希尔伯特变换)
 */
#pragma once

#include <QObject>
#include <QVector>

class SignalSynchronizer10 : public QObject {
    Q_OBJECT

public:
    /** @brief Synchronization result */
    struct SyncResult {
        double delaySamples = 0.0;   // Subsample delay in samples
        double delaySeconds = 0.0;
        double correlationPeak = 0.0;
        double snr = 0.0;
        int integerDelay = 0;        // Integer part of delay
        bool valid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSyncs = 0;
        int signalLength = 0;
        double avgCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer10(QObject *parent = nullptr);
    ~SignalSynchronizer10() override;

    void setSampleRate(double rate);

    /** @brief Synchronize two signals (reference and delayed) */
    SyncResult synchronize(const QVector<double>& reference,
                            const QVector<double>& delayed);

    /** @brief Compute cross-correlation via FFT */
    QVector<double> crossCorrelation(const QVector<double>& a,
                                      const QVector<double>& b) const;

    /** @brief Parabolic interpolation for subsample peak */
    double interpolatePeak(const QVector<double>& correlation,
                            int peakIndex) const;

    /** @brief Align delayed signal to reference */
    QVector<double> alignSignals(const QVector<double>& delayed,
                                  double delaySamples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void syncDone(double delaySamples, double correlation, double timeMs);

private:
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_corrSum = 0.0;

    /** @brief In-place FFT for cross-correlation */
    void fftInPlace(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Reverse bits */
    int reverseBits(int val, int bits) const;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Linear fractional delay interpolation */
    double fractionalDelay(const QVector<double>& sig,
                            double index) const;
};
