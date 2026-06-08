/**
 * @file Correlator5.h
 * @brief 互相关器(频域GCC相位变换+相干性门控干扰抑制) — Cross-Correlator with Frequency-Domain GCC with Phase Transform and Coherence-Gated Interference Rejection
 *
 * 功能: 实现互相关计算，采用频域广义互相关(GCC)加相位变换(PHAT)，
 *       结合相干性门控实现干扰抑制和时延估计。
 *
 * 协作: SpectralGate5(谱门控) / WienerFilter3(Wiener滤波) / Beamformer2(波束形成)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 互相关器(GCC-PHAT+相干性门控)
 */
class Correlator5 : public QObject {
    Q_OBJECT

public:
    /** @brief Correlation result */
    struct CorrResult {
        QVector<double> correlation;
        int peakLag = 0;
        double peakValue = 0.0;
        double delaySamples = 0.0;
        double coherence = 0.0;
        double snr = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int fftSize = 0;
        int peakLag = 0;
        double peakValue = 0.0;
        double coherence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator5(QObject *parent = nullptr);
    ~Correlator5() override;

    /** @brief Set parameters: FFT size, coherence threshold */
    void setParameters(int fftSize = 1024, double coherenceThreshold = 0.5);

    /** @brief Compute GCC-PHAT cross-correlation */
    CorrResult gccPhat(const QVector<double>& signal1,
                        const QVector<double>& signal2);

    /** @brief Compute standard cross-correlation (time domain) */
    QVector<double> crossCorrelate(const QVector<double>& signal1,
                                     const QVector<double>& signal2) const;

    /** @brief Compute magnitude-squared coherence */
    QVector<double> coherence(const QVector<double>& signal1,
                                const QVector<double>& signal2);

    /** @brief Estimate time delay from correlation peak */
    double estimateDelay(const QVector<double>& correlation) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int peakLag, double peakValue, double timeMs);

private:
    int m_fftSize = 1024;
    double m_coherenceThreshold = 0.5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Inverse FFT */
    void ifft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Apply PHAT weighting to cross-spectrum */
    void applyPhat(QVector<double>& crossRe, QVector<double>& crossIm) const;

    /** @brief Apply coherence gating to cross-spectrum */
    void applyCoherenceGate(QVector<double>& crossRe, QVector<double>& crossIm,
                              const QVector<double>& coh) const;

    /** @brief Compute cross-power spectrum */
    void crossSpectrum(const QVector<double>& re1, const QVector<double>& im1,
                         const QVector<double>& re2, const QVector<double>& im2,
                         QVector<double>& crossRe, QVector<double>& crossIm) const;
};
