/**
 * @file Correlator7.h
 * @brief 互相关器(广义互相关相位变换+相干加权时延估计) — Cross-Correlator with Generalized Cross-Correlation Phase Transform and Coherence-Weighted Time Delay Estimation
 *
 * 功能: 实现互相关器(Cross-Correlator)，使用广义互相关相位变换(GCC-PHAT)
 *       (Generalized Cross-Correlation Phase Transform)获得尖锐相关峰，
 *       相干加权(coherence-weighted)时延估计提高鲁棒性。
 *
 * 协作: SpectralGate6(频谱门控) / KalmanFilter4(卡尔曼滤波) / IIRFilter3(IIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 互相关器(GCC-PHAT+相干加权时延估计)
 */
class Correlator7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int numCorrelations = 0;
        double lastDelay = 0.0;
        double lastCoherence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator7(QObject *parent = nullptr);
    ~Correlator7() override;

    /** @brief Set FFT size for correlation */
    void setFFTSize(int size);

    /** @brief Set max search lag for delay estimation */
    void setMaxLag(int maxLag);

    /** @brief Compute GCC-PHAT cross-correlation */
    QVector<double> gccPhat(const QVector<double>& x,
                             const QVector<double>& y);

    /** @brief Estimate time delay between two signals (in samples) */
    double estimateDelay(const QVector<double>& x,
                          const QVector<double>& y);

    /** @brief Compute magnitude-squared coherence */
    QVector<double> coherence(const QVector<double>& x,
                               const QVector<double>& y);

    /** @brief Coherence-weighted delay estimation */
    double coherenceWeightedDelay(const QVector<double>& x,
                                   const QVector<double>& y);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int fftSize, double peakLag, double timeMs);
    void delayEstimated(double delay, double coherence, double timeMs);

private:
    int m_fftSize = 1024;
    int m_maxLag = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place inverse FFT */
    void ifft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Zero-pad signal to FFT size */
    QVector<double> padSignal(const QVector<double>& sig) const;

    /** @brief Compute cross-spectrum with PHAT weighting */
    void crossSpectrumPHAT(const QVector<double>& xRe,
                            const QVector<double>& xIm,
                            const QVector<double>& yRe,
                            const QVector<double>& yIm,
                            QVector<double>& outRe,
                            QVector<double>& outIm) const;

    /** @brief Find peak index in correlation with parabolic interpolation */
    double findPeak(const QVector<double>& corr) const;
};
