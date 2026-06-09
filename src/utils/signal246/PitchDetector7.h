/**
 * @file PitchDetector7.h
 * @brief 音高检测(自相关函数+YIN差分函数抛物线插值精化) — Pitch Detector with Autocorrelation Function and YIN Difference Function with Parabolic Interpolation Refinement
 *
 * 功能: 实现音高检测(Pitch Detection)，结合自相关函数(autocorrelation function)
 *       和YIN差分函数(YIN difference function)，使用抛物线插值(parabolic
 *       interpolation)对基频估计进行亚采样级精化。
 *
 * 协作: PitchShifter6(变调) / BeatDetector5(节拍检测) / SpectralAnalyzer7(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 音高检测(自相关函数+YIN差分函数抛物线插值精化)
 */
class PitchDetector7 : public QObject {
    Q_OBJECT

public:
    /** @brief Pitch detection result */
    struct PitchResult {
        double frequency = 0.0;     // Hz
        double confidence = 0.0;    // 0..1
        int period = 0;             // samples
        double refinedPeriod = 0.0; // sub-sample refined period
        bool voiced = false;        // voiced/unvoiced decision
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        double sampleRate = 44100.0;
        int blockSize = 0;
        int numVoiced = 0;
        int numUnvoiced = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector7(QObject *parent = nullptr);
    ~PitchDetector7() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set minimum detectable frequency (Hz) */
    void setMinFrequency(double freq);

    /** @brief Set maximum detectable frequency (Hz) */
    void setMaxFrequency(double freq);

    /** @brief Set voiced/unvoiced threshold (0..1) */
    void setVoicedThreshold(double threshold);

    /** @brief Detect pitch from a block of samples */
    PitchResult detect(const QVector<double>& samples);

    /** @brief Compute autocorrelation function */
    QVector<double> autocorrelation(const QVector<double>& samples) const;

    /** @brief Compute YIN difference function */
    QVector<double> yinDifference(const QVector<double>& samples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double frequency, double confidence, bool voiced);

private:
    double m_sampleRate = 44100.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
    double m_voicedThreshold = 0.15;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cumulative mean normalized difference (YIN step 3) */
    QVector<double> cumulativeMeanNormalized(const QVector<double>& diff) const;

    /** @brief Parabolic interpolation around minimum for sub-sample precision */
    double parabolicRefine(const QVector<double>& data, int minIdx) const;

    /** @brief Absolute threshold search (YIN step 4) */
    int findFirstMinimum(const QVector<double>& cmndf, int minLag, int maxLag) const;
};
