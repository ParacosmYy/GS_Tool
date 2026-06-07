/**
 * @file Correlator3.h
 * @brief 互相关器(GCC-PHAT广义互相关+相干性门控) — Cross-Correlator with GCC-PHAT (Generalized Cross-Correlation with Phase Transform) and Coherence Gating
 *
 * 功能: 实现广义互相关GCC-PHAT时延估计，支持相干性门控、
 *       互功率谱计算和峰值检测。
 *
 * 协作: FftEngine9(FFT引擎) / WinogradFFT6(Winograd-FFT) / FilterBank5(滤波器组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 互相关器(GCC-PHAT广义互相关+相干性门控)
 */
class Correlator3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalCorrelations = 0;
        int frameSize = 0;
        double lastDelay = 0.0;
        double lastCoherence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator3(QObject *parent = nullptr);
    ~Correlator3() override;

    void setFrameSize(int size);
    void setSampleRate(double rate);
    void setCoherenceThreshold(double threshold);

    /** @brief Compute GCC-PHAT and return delay in samples */
    double gccPhat(const QVector<double>& x, const QVector<double>& y);

    /** @brief Compute cross-power spectrum (interleaved real/imag) */
    QVector<double> crossPowerSpectrum(const QVector<double>& xReal,
                                        const QVector<double>& xImag,
                                        const QVector<double>& yReal,
                                        const QVector<double>& yImag) const;

    /** @brief Apply phase transform weighting */
    QVector<double> phaseTransform(const QVector<double>& cpsReal,
                                    const QVector<double>& cpsImag) const;

    /** @brief Compute magnitude-squared coherence */
    QVector<double> coherence(const QVector<double>& x,
                               const QVector<double>& y) const;

    /** @brief Apply coherence gating: zero out low-coherence bins */
    void applyCoherenceGate(QVector<double>& gcc,
                             const QVector<double>& coh) const;

    /** @brief Find peak in correlation with parabolic interpolation */
    double findPeak(const QVector<double>& corr) const;

    /** @brief Simple radix-2 FFT for internal use */
    static void fft(QVector<double>& real, QVector<double>& imag, bool inverse);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(double delaySamples, double coherence, double timeMs);

private:
    int m_frameSize = 512;
    double m_sampleRate = 44100.0;
    double m_coherenceThreshold = 0.5;

    // Auto-power accumulators for coherence estimation
    QVector<double> m_autoPowX;
    QVector<double> m_autoPowY;
    double m_alpha = 0.95;     // exponential smoothing

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute auto-power spectrum */
    QVector<double> autoPower(const QVector<double>& signal) const;
};
