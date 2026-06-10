/**
 * @file Periodogram7.h
 * @brief 周期图(Lomb-Scargle非均匀采样时间序列假警概率估计) — Periodogram with Lomb-Scargle Method for Unevenly Sampled Time Series and False Alarm Probability Estimation
 *
 * 功能: 实现周期图(periodogram)，采用Lomb-Scargle方法(Lomb-Scargle method)
 *       处理非均匀采样时间序列(unevenly sampled time series)，并提供假警概率
 *       估计(false alarm probability estimation)评估周期显著性。
 *
 * 协作: PowerSpectrum8(功率谱) / SplitRadixFFT9(FFT) / WaveletDenoise8(小波降噪)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(Lomb-Scargle非均匀采样时间序列假警概率估计)
 */
class Periodogram7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numTimePoints = 0;
        int numFreqBins = 0;
        double maxPower = 0.0;
        double dominantFrequency = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Peak detection result */
    struct Peak {
        double frequency = 0.0;
        double power = 0.0;
        double falseAlarmProb = 0.0;
        double significanceSigma = 0.0;
    };

    explicit Periodogram7(QObject *parent = nullptr);
    ~Periodogram7() override;

    /** @brief Set frequency range [minFreq, maxFreq] */
    void setFrequencyRange(double minFreq, double maxFreq);

    /** @brief Set number of frequency bins */
    void setNumFreqBins(int bins);

    /** @brief Compute Lomb-Scargle periodogram from unevenly sampled data */
    QVector<double> compute(const QVector<double>& times,
                            const QVector<double>& values) const;

    /** @brief Compute false alarm probability for a given power level */
    double falseAlarmProbability(double power, int n) const;

    /** @brief Detect significant peaks above a threshold */
    QVector<Peak> detectPeaks(const QVector<double>& frequencies,
                               const QVector<double>& power,
                               double fapThreshold = 0.01) const;

    /** @brief Get frequency axis for last computation */
    QVector<double> frequencyAxis() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void periodogramComputed(int numPoints, int numBins, double maxPower, double timeMs);
    void peakDetected(double freq, double power, double fap);

private:
    double m_minFreq = 0.0;
    double m_maxFreq = 1.0;
    int m_numBins = 1024;

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Generate linearly spaced frequency grid */
    QVector<double> linspace(double start, double end, int n) const;

    /** @brief Compute Lomb-Scargle power at a single frequency */
    double lombScarglePower(double freq, const QVector<double>& t,
                            const QVector<double>& y, double yMean) const;

    /** @brief Estimate noise level from median of periodogram */
    double estimateNoiseLevel(const QVector<double>& power) const;

    /** @brief Convert FAP to equivalent Gaussian sigma */
    double fapToSigma(double fap) const;

    /** @brief Incomplete gamma function for FAP computation */
    double incompleteGamma(double a, double x) const;

    /** @brief Log-gamma function (Stirling's approximation) */
    double logGamma(double x) const;
};
