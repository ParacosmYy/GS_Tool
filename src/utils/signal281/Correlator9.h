/**
 * @file Correlator9.h
 * @brief 相关器(最大似然时延估计与Cramer-Rao界约束的最优统计时延估计) — Correlator with Maximum Likelihood Time Delay Estimation and Cramer-Rao Bound for Optimal Statistical Delay Estimation
 *
 * 功能: 实现相关器(correlator)，采用最大似然时延估计(maximum likelihood time delay estimation)
 *       与Cramer-Rao界(Cramer-Rao bound)实现最优统计时延估计(optimal statistical delay estimation)。
 *
 * 协作: MatchedFilter8(匹配滤波) / WienerFilter6(Wiener滤波) / SpectralGate8(频谱门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相关器(最大似然时延估计与Cramer-Rao界)
 */
class Correlator9 : public QObject {
    Q_OBJECT

public:
    /** @brief Delay estimation result */
    struct DelayResult {
        double delaySamples = 0.0;      // Fractional delay in samples
        double delaySeconds = 0.0;
        double correlationPeak = 0.0;
        double snr = 0.0;
        double cramerRaoBound = 0.0;    // CRLB for delay estimate
        double confidence = 0.0;
        int searchIndex = 0;
        bool valid = false;
    };

    /** @brief Cross-correlation result */
    struct CorrelationResult {
        QVector<double> correlation;
        int lagMax = 0;
        double peakValue = 0.0;
        double normalizedPeak = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        double avgProcessingTimeMs = 0.0;
        double avgSnr = 0.0;
    };

    explicit Correlator9(QObject *parent = nullptr);
    ~Correlator9() override;

    /** @brief Set sample rate for time delay conversion */
    void setSampleRate(double rate);

    /** @brief Set search range for delay (in samples) */
    void setSearchRange(int minLag, int maxLag);

    /** @brief Set interpolation order for sub-sample resolution */
    void setInterpolationOrder(int order);

    /** @brief Estimate time delay between two signals via ML */
    DelayResult estimateDelay(const QVector<double>& ref,
                               const QVector<double>& observed);

    /** @brief Compute cross-correlation function */
    CorrelationResult crossCorrelate(const QVector<double>& a,
                                      const QVector<double>& b) const;

    /** @brief Compute Cramer-Rao lower bound for delay estimate */
    double cramerRaoBound(const QVector<double>& signal, double snr) const;

    /** @brief Parabolic interpolation for sub-sample peak */
    double parabolicInterpolation(const QVector<double>& corr, int peakIdx) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void delayEstimated(double delay, double crlb, double timeMs);
    void correlationDone(int lag, double peak, double timeMs);

private:
    double m_sampleRate = 48000.0;
    int m_minLag = 0;
    int m_maxLag = 256;
    int m_interpOrder = 3;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_snrSum = 0.0;

    /** @brief Estimate SNR from signal and correlation */
    double estimateSNR(const QVector<double>& signal,
                        double noiseFloor) const;

    /** @brief Gaussian interpolation for fractional peak */
    double gaussianInterpolation(const QVector<double>& corr, int peakIdx) const;

    /** @brief Compute signal bandwidth for CRLB */
    double signalBandwidth(const QVector<double>& signal) const;

    /** @brief Apply window to reduce spectral leakage */
    QVector<double> applyWindow(const QVector<double>& data) const;
};
