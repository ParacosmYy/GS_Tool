/**
 * @file PitchDetector5.h
 * @brief 基频检测器(归一化互相关+YIN累积均值差+抛物线精化) — Pitch Detector with Normalized Cross-Correlation and YIN Cumulative Mean Difference with Parabolic Refinement
 *
 * 功能: 实现基频检测，结合归一化互相关(NCC)与YIN累积均值差函数(CMDF)，
 *       使用抛物线插值精化基频估计。
 *
 * 协作: Autocorrelation3(自相关) / Cepstrum4(倒谱) / HarmonicTracker6(谐波跟踪)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 基频检测器(NCC+YIN CMDF+抛物线精化)
 */
class PitchDetector5 : public QObject {
    Q_OBJECT

public:
    /** @brief Pitch detection result */
    struct PitchResult {
        double frequency = 0.0;     // Hz
        double confidence = 0.0;    // [0..1]
        bool voiced = false;
        int period = 0;             // samples
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        int sampleRate = 44100;
        double minFreq = 50.0;
        double maxFreq = 2000.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector5(QObject *parent = nullptr);
    ~PitchDetector5() override;

    /** @brief Set detection parameters */
    void setParameters(int sampleRate, int frameSize,
                       double minFreq = 50.0, double maxFreq = 2000.0);

    /** @brief Detect pitch from audio frame */
    PitchResult detect(const QVector<double>& frame) const;

    /** @brief Compute normalized cross-correlation function */
    QVector<double> ncc(const QVector<double>& frame) const;

    /** @brief Compute YIN cumulative mean difference function */
    QVector<double> cmdf(const QVector<double>& frame) const;

    /** @brief Parabolic refinement around minimum/peak index */
    double parabolicRefine(const QVector<double>& func, int idx) const;

    /** @brief Detect pitch using NCC method */
    PitchResult detectNCC(const QVector<double>& frame) const;

    /** @brief Detect pitch using YIN method */
    PitchResult detectYIN(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double freq, double confidence, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_frameSize = 2048;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
    int m_minLag = 0;
    int m_maxLag = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update lag bounds from frequency limits */
    void updateLagBounds();
};
