/**
 * @file Correlator8.h
 * @brief 相关器(广义互相关与相位变换加权噪声信道时延估计) — Correlator with Generalized Cross-Correlation and Phase Transform Weighting for Time Delay Estimation in Noisy Channels
 *
 * 功能: 实现相关器(correlator)，采用广义互相关(generalized cross-correlation)
 *       和相位变换加权(phase transform weighting)实现噪声信道时延估计
 *       (time delay estimation in noisy channels)。
 *
 * 协作: SpectralGate7(谱门控) / FftEngine8(FFT引擎) / BruunFFT10(Bruun FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相关器(广义互相关与相位变换加权噪声信道时延估计)
 */
class Correlator8 : public QObject {
    Q_OBJECT

public:
    /** @brief Weighting method for generalized cross-correlation */
    enum WeightingMethod {
        Standard = 0,    // No weighting (standard cross-correlation)
        SCC,             // Smoothed coherence transform
        PHAT,            // Phase transform
        ML,              // Maximum likelihood
        Eckart           // Eckart filter
    };
    Q_ENUM(WeightingMethod)

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int estimatedLag = 0;
        double peakCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator8(QObject *parent = nullptr);
    ~Correlator8() override;

    /** @brief Set weighting method */
    void setMethod(WeightingMethod method);

    /** @brief Compute cross-correlation between two signals */
    QVector<double> correlate(const QVector<double>& signal1,
                               const QVector<double>& signal2) const;

    /** @brief Estimate time delay (lag) between two signals */
    int estimateDelay(const QVector<double>& signal1,
                       const QVector<double>& signal2) const;

    /** @brief Compute GCC with specified weighting */
    QVector<double> gcc(const QVector<double>& signal1,
                         const QVector<double>& signal2,
                         WeightingMethod method) const;

    /** @brief Compute auto-correlation */
    QVector<double> autoCorrelate(const QVector<double>& signal) const;

    /** @brief Get correlation peak value and lag */
    QPair<double, int> findPeak(const QVector<double>& correlation) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationComputed(int length, int lag, double peak, double timeMs);

private:
    WeightingMethod m_method = PHAT;
    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief FFT using radix-2 Cooley-Tukey */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Inverse FFT */
    void ifft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Apply PHAT weighting to cross-spectrum */
    void applyPHAT(QVector<double>& crossRe, QVector<double>& crossIm) const;

    /** @brief Apply ML weighting */
    void applyML(const QVector<double>& spec1, const QVector<double>& spec2,
                  QVector<double>& crossRe, QVector<double>& crossIm) const;

    /** @brief Compute power spectrum */
    QVector<double> powerSpectrum(const QVector<double>& re,
                                    const QVector<double>& im) const;

    /** @brief Pad to next power of 2 */
    static int nextPow2(int n);
};
