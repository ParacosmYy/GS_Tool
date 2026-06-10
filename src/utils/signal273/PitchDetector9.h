/**
 * @file PitchDetector9.h
 * @brief 基频检测器(自相关YIN算法与累积均值归一化差分鲁棒F0估计) — Pitch Detector with Autocorrelation-Based YIN Algorithm and Cumulative Mean Normalized Difference for Robust F0 Estimation
 *
 * 功能: 实现基频检测器(Pitch detector)，采用自相关YIN算法(autocorrelation-based YIN algorithm)
 *       与累积均值归一化差分(cumulative mean normalized difference)实现鲁棒F0估计(robust F0 estimation)。
 *
 * 协作: Autocorrelation6(自相关) / onsetDetector7(起音检测) / EnvelopeDetector5(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 基频检测器(自相关YIN算法与累积均值归一化差分鲁棒F0估计)
 */
class PitchDetector9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        double lastPitchHz = 0.0;
        double lastConfidence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector9(QObject *parent = nullptr);
    ~PitchDetector9() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set minimum detectable frequency (Hz) */
    void setMinFrequency(double hz);

    /** @brief Set maximum detectable frequency (Hz) */
    void setMaxFrequency(double hz);

    /** @brief Set YIN threshold for absolute minimum (0.0-1.0) */
    void setThreshold(double threshold);

    /** @brief Detect pitch from audio samples, returns frequency in Hz (0 = no pitch) */
    double detect(const QVector<double>& samples);

    /** @brief Detect pitch with confidence, returns {freqHz, confidence} */
    QPair<double, double> detectWithConfidence(const QVector<double>& samples);

    /** @brief Get the cumulative mean normalized difference function */
    QVector<double> differenceFunction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double freqHz, double confidence, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
    double m_threshold = 0.15;

    QVector<double> m_cmnDiff;   // cumulative mean normalized difference

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute difference function D(tau) = sum (x[j]-x[j+tau])^2 */
    QVector<double> computeDifference(const QVector<double>& x, int maxTau) const;

    /** @brief Compute cumulative mean normalized difference d'(tau) */
    QVector<double> cumulativeMeanNorm(const QVector<double>& diff) const;

    /** @brief Find absolute minimum with threshold */
    int findAbsMin(const QVector<double>& cmnd) const;

    /** @brief Parabolic interpolation for sub-sample accuracy */
    double parabolicInterpolation(const QVector<double>& cmnd, int tau) const;
};
