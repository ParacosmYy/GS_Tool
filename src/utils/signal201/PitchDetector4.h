/**
 * @file PitchDetector4.h
 * @brief 基频检测(YIN算法:累积均值归一化差分+绝对阈值) — Pitch Detection via YIN Algorithm with Cumulative Mean Normalized Difference and Absolute Threshold
 *
 * 功能: 实现YIN基频检测算法，支持累积均值归一化差分函数、
 *       绝对阈值检测和抛物线插值精化。
 *
 * 协作: FftEngine(FFT引擎) / WindowFunction(窗函数) / Goertzel4(Goertzel算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 基频检测(YIN算法+绝对阈值)
 */
class PitchDetector4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalDetections = 0;
        double lastPitch = 0.0;
        double lastConfidence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Pitch detection result */
    struct PitchResult {
        double frequency = 0.0;
        double period = 0.0;
        double confidence = 0.0;
        bool voiced = false;
    };

    explicit PitchDetector4(QObject *parent = nullptr);
    ~PitchDetector4() override;

    void setSampleRate(double rate);
    void setMinFrequency(double freq);
    void setMaxFrequency(double freq);
    void setThreshold(double threshold);

    /** @brief Detect pitch from time-domain signal */
    PitchResult detect(const QVector<double>& signal);

    /** @brief Compute difference function d(tau) */
    QVector<double> differenceFunction(const QVector<double>& signal, int tauMax) const;

    /** @brief Cumulative mean normalized difference function d'(tau) */
    QVector<double> cumulativeMeanNormalized(const QVector<double>& diff) const;

    /** @brief Absolute threshold search for period estimate */
    int absoluteThreshold(const QVector<double>& cmndf, double threshold) const;

    /** @brief Parabolic interpolation around detected tau */
    double parabolicInterpolation(const QVector<double>& cmndf, int tau) const;

    /** @brief Detect pitch on overlapping frames */
    QVector<PitchResult> detectFrames(const QVector<double>& signal, int frameSize, int hopSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double frequency, double confidence, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
    double m_threshold = 0.15;

    Stats m_stats;
    double m_timeSum = 0.0;

    int tauMin() const;
    int tauMax(int signalLen) const;
};
