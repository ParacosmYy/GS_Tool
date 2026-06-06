/**
 * @file PitchDetector3.h
 * @brief 基频检测(自相关+抛物线插值+有声/无声判决) — Pitch Detection via Autocorrelation with Parabolic Interpolation and Voicing Decision
 *
 * 功能: 实现基于自相关的基频检测算法，支持抛物线插值精化、
 *       有声/无声判决、倍频/半频校正和多帧平滑跟踪。
 *
 * 协作: WindowFunction5(窗函数) / GoertzelFilter4(Goertzel) / PitchShifter3(变调)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 基频检测器(自相关+抛物线插值+有声判决)
 */
class PitchDetector3 : public QObject {
    Q_OBJECT

public:
    /** @brief Pitch detection result */
    struct PitchResult {
        double frequency = 0.0;     ///< Detected pitch (Hz), 0 = unvoiced
        double confidence = 0.0;    ///< Detection confidence [0,1]
        bool isVoiced = false;      ///< Voicing decision
        double period = 0.0;        ///< Period in samples
        double clarity = 0.0;       ///< Peak clarity measure
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int sampleRate = 0;
        double minPitch = 0.0;
        double maxPitch = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector3(int sampleRate = 44100, QObject *parent = nullptr);
    ~PitchDetector3() override;

    void setSampleRate(int sr);
    void setPitchRange(double minHz, double maxHz);
    void setVoicingThreshold(double threshold);
    void setClarityThreshold(double threshold);

    /** @brief 检测单帧基频 */
    PitchResult detect(const QVector<double>& frame) const;

    /** @brief 检测多帧基频序列 */
    QVector<PitchResult> detectMulti(const QVector<QVector<double>>& frames) const;

    /** @brief Compute autocorrelation function (biased) */
    QVector<double> autocorrelation(const QVector<double>& frame) const;

    /** @brief Parabolic interpolation around peak */
    double parabolicInterpolation(const QVector<double>& acf,
                                   int peakIndex) const;

    /** @brief Voicing decision from autocorrelation */
    bool decideVoiced(const QVector<double>& acf,
                      const QVector<double>& frame) const;

    /** @brief Octave correction (prevent half/double errors) */
    double octaveCorrect(double freq, const QVector<double>& acf,
                          double samplePeriod) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double frequency, bool voiced, double confidence);

private:
    int m_sampleRate = 44100;
    double m_minPitch = 50.0;
    double m_maxPitch = 800.0;
    double m_voiceThreshold = 0.3;
    double m_clarityThreshold = 0.2;

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Find first significant peak in ACF in pitch range */
    int findPeakInRange(const QVector<double>& acf,
                        int minLag, int maxLag) const;

    /** @brief Compute RMS energy of frame */
    double rmsEnergy(const QVector<double>& frame) const;

    /** @brief Compute zero crossing rate */
    double zeroCrossingRate(const QVector<double>& frame) const;
};
