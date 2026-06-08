/**
 * @file SignalSynchronizer5.h
 * @brief 信号同步器(广义互相关+MUSIC伪谱多源TDOA估计) — Signal Synchronizer with Generalized Cross-Correlation and MUSIC Pseudospectrum for Multi-Source TDOA
 *
 * 功能: 实现多源信号时间差(TDOA)估计，集成广义互相关(GCC)和MUSIC伪谱算法，
 *       支持多麦克风阵列的多源到达时间差同步。
 *
 * 协作: CrossCorrelator7(互相关) / SpectrumAnalyzer6(频谱分析) / Beamformer4(波束形成)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号同步器(GCC+MUSIC多源TDOA)
 */
class SignalSynchronizer5 : public QObject {
    Q_OBJECT

public:
    /** @brief TDOA estimation result */
    struct TDOAResult {
        double delaySamples = 0.0;
        double delaySeconds = 0.0;
        double confidence = 0.0;
        int peakIndex = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int numSources = 0;
        int numSensors = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer5(QObject *parent = nullptr);
    ~SignalSynchronizer5() override;

    /** @brief Set parameters: sample rate, FFT size, max sources */
    void setParameters(double sampleRate, int fftSize, int maxSources = 3);

    /** @brief Estimate TDOA using generalized cross-correlation (GCC-PHAT) */
    TDOAResult estimateGCC(const QVector<double>& ref,
                            const QVector<double>& test) const;

    /** @brief Multi-source TDOA estimation via MUSIC pseudospectrum */
    QVector<TDOAResult> estimateMultiTDOA(
        const QVector<QVector<double>>& sensorSignals) const;

    /** @brief Compute MUSIC pseudospectrum for delay estimation */
    QVector<double> musicPseudospectrum(
        const QVector<QVector<double>>& signals, int numSources) const;

    /** @brief Synchronize multiple signals to reference */
    QVector<QVector<double>> synchronize(
        const QVector<QVector<double>>& signals) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void synchronizationCompleted(int sources, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 1024;
    int m_maxSources = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute FFT of signal (radix-2 DIT) */
    void computeFFT(QVector<double>& re, QVector<double>& im) const;

    /** @brief Compute inverse FFT */
    void computeIFFT(QVector<double>& re, QVector<double>& im) const;

    /** @brief Compute cross-power spectrum with PHAT weighting */
    void crossPowerPHAT(const QVector<double>& refRe,
                          const QVector<double>& refIm,
                          const QVector<double>& testRe,
                          const QVector<double>& testIm,
                          QVector<double>& outRe,
                          QVector<double>& outIm) const;

    /** @brief Parabolic interpolation for sub-sample TDOA */
    double parabolicInterpolation(const QVector<double>& corr,
                                    int peakIdx) const;

    /** @brief Eigenvalue decomposition of correlation matrix (Jacobi) */
    void jacobiEigen(QVector<QVector<double>>& A,
                       QVector<double>& eigenvalues,
                       QVector<QVector<double>>& eigenvectors,
                       int maxIter = 100) const;

    /** @brief Build spatial correlation matrix */
    QVector<QVector<double>> buildCorrelationMatrix(
        const QVector<QVector<double>>& freqData) const;
};
