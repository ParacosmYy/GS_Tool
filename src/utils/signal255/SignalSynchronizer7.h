/**
 * @file SignalSynchronizer7.h
 * @brief 信号同步器(广义互相关GCC+抛物线峰值拟合亚采样插值) — Signal Synchronizer with Generalized Cross-Correlation and Sub-Sample Interpolation via Parabolic Peak Fitting
 *
 * 功能: 实现信号同步器(Signal Synchronizer)，使用广义互相关(GCC,
 *       Generalized Cross-Correlation)计算信号间时延，通过抛物线
 *       峰值拟合(parabolic peak fitting)实现亚采样精度(sub-sample
 *       precision)的时间延迟估计。
 *
 * 协作: CrossCorrelator5(互相关) / FftEngine8(FFT引擎) / WienerFilter6(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号同步器(GCC+抛物线亚采样插值)
 */
class SignalSynchronizer7 : public QObject {
    Q_OBJECT

public:
    /** @brief GCC weighting function */
    enum GCCWeighting {
        Standard = 0,   // Unweighted cross-correlation
        PHAT = 1,       // Phase Transform
        Scott = 2,      // SCOT weighting
        ML = 3          // Maximum Likelihood
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numSynchronizations = 0;
        double lastDelay = 0.0;
        double lastCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer7(QObject *parent = nullptr);
    ~SignalSynchronizer7() override;

    /** @brief Set GCC weighting function */
    void setWeighting(GCCWeighting w);

    /** @brief Compute time delay between reference and delayed signal */
    double synchronize(const QVector<double>& reference,
                       const QVector<double>& delayed);

    /** @brief Get cross-correlation function from last sync */
    QVector<double> correlationFunction() const;

    /** @brief Get interpolated peak delay with sub-sample precision */
    double subSampleDelay() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void synchronizationCompleted(double delaySamples, double peakCorrelation, double timeMs);

private:
    GCCWeighting m_weighting = Standard;
    int m_corrSize = 0;

    QVector<double> m_corr;       // Cross-correlation result
    double m_integerDelay = 0.0;  // Integer sample delay
    double m_subSampleDelay = 0.0;// Sub-sample refined delay
    double m_peakCorr = 0.0;     // Peak correlation value

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute cross-power spectrum for GCC */
    void crossPowerSpectrum(const QVector<double>& ref,
                            const QVector<double>& del,
                            QVector<double>& powerRe,
                            QVector<double>& powerIm) const;

    /** @brief Apply GCC weighting to cross-power spectrum */
    void applyWeighting(QVector<double>& re, QVector<double>& im) const;

    /** @brief Find peak index in correlation */
    int findPeak(const QVector<double>& corr) const;

    /** @brief Refine peak with parabolic interpolation */
    double parabolicPeak(const QVector<double>& corr, int peakIdx) const;

    /** @brief Simple radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief Next power of 2 */
    static int nextPow2(int n);
};
