/**
 * @file SignalSynchronizer6.h
 * @brief 信号同步器(交叉谱相位估计+分数延迟插值+时间对齐) — Signal Synchronizer with Cross-Spectral Phase Estimation and Fractional Delay Interpolation for Time Alignment
 *
 * 功能: 实现信号同步器(Signal synchronizer)，采用交叉谱相位估计(cross-spectral phase estimation)
 *       在频域计算两信号间的相位差，利用分数延迟插值(fractional delay interpolation)通过
 *       sinc重采样实现亚采样级时间对齐(sub-sample time alignment)。
 *
 * 协作: CICFilter4(CIC滤波器) / FIRFilter5(FIR滤波器) / PLL5(锁相环)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号同步器(交叉谱相位估计+分数延迟插值+时间对齐)
 */
class SignalSynchronizer6 : public QObject {
    Q_OBJECT

public:
    /** @brief Synchronization result */
    struct SyncResult {
        double delaySamples = 0.0;   // estimated fractional delay
        double coherence = 0.0;      // cross-spectral coherence
        double timeOffsetMs = 0.0;   // time offset in ms
        double confidence = 0.0;     // 0..1 confidence score
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numSynchronized = 0;
        double avgCoherence = 0.0;
        double avgDelaySamples = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer6(QObject *parent = nullptr);
    ~SignalSynchronizer6() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double hz);

    /** @brief Set FFT size for cross-spectral estimation */
    void setFftSize(int size);

    /** @brief Estimate delay between reference and delayed signal */
    SyncResult estimateDelay(const QVector<double>& reference,
                             const QVector<double>& delayed);

    /** @brief Apply fractional delay to align signal with reference */
    QVector<double> applyDelay(const QVector<double>& signal, double delaySamples);

    /** @brief Full sync: estimate delay and align */
    QVector<double> synchronize(const QVector<double>& reference,
                                const QVector<double>& delayed);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void delayEstimated(double delaySamples, double coherence, double timeMs);
    void syncCompleted(double delaySamples, double totalTimeMs);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Windowed FFT (real input -> complex output as re/im pairs) */
    void computeFFT(const QVector<double>& input,
                    QVector<double>& re, QVector<double>& im) const;

    /** @brief Cross-power spectrum and coherence */
    void crossSpectrum(const QVector<double>& refRe, const QVector<double>& refIm,
                       const QVector<double>& sigRe, const QVector<double>& sigIm,
                       QVector<double>& crossRe, QVector<double>& crossIm,
                       QVector<double>& coherence) const;

    /** @brief Fractional delay via sinc interpolation */
    double sincInterpolate(const QVector<double>& signal, double index) const;

    /** @brief Lanczos windowed sinc kernel */
    double lanczosKernel(double x, int a) const;
};
