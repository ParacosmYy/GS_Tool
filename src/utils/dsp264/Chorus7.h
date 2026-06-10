/**
 * @file Chorus7.h
 * @brief 合唱效果器(多抽头调制延迟线与相位偏移LFO立体声空间扩展增厚) — Chorus with Multi-tap Modulated Delay Lines and Stereo Spatial Spread via Phase-offset LFO for Thickening Effects
 *
 * 功能: 实现合唱效果器(Chorus effect)，采用多抽头调制延迟线(multi-tap modulated delay lines)
 *       和相位偏移LFO(phase-offset LFO)实现立体声空间扩展(stereo spatial spread)增厚效果
 *       (thickening effects)。
 *
 * 协作: Flanger6(Flanger效果) / Reverb5(混响) / Delay4(延迟效果)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 合唱效果器(多抽头调制延迟线与相位偏移LFO立体声空间扩展增厚)
 */
class Chorus7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numTaps = 0;
        double baseDelayMs = 0.0;
        double depthMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus7(QObject *parent = nullptr);
    ~Chorus7() override;

    /** @brief Set chorus parameters */
    void setParameters(double baseDelayMs = 7.0, double depthMs = 3.0,
                       double rateHz = 0.5, int numTaps = 3,
                       double mix = 0.5, double sampleRate = 44100.0);

    /** @brief Process mono input, returns stereo output (interleaved L/R) */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process mono input to mono chorus output */
    QVector<double> processMono(const QVector<double>& input);

    /** @brief Reset delay lines and LFO phase */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingComplete(int numSamples, double timeMs);

private:
    double m_baseDelayMs = 7.0;
    double m_depthMs = 3.0;
    double m_rateHz = 0.5;
    int m_numTaps = 3;
    double m_mix = 0.5;
    double m_sampleRate = 44100.0;

    // Delay line: circular buffer per tap
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_writePos;
    int m_maxDelaySamples = 0;

    // LFO state per tap (phase offset)
    QVector<double> m_lfoPhase;
    double m_lfoIncrement = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize delay lines and LFO */
    void initDelayLines();

    /** @brief Compute LFO value for given tap (sine wave with phase offset) */
    double lfoValue(int tap) const;

    /** @brief Read from delay line with fractional sample position (linear interp) */
    double readInterpolated(int tap, double samplePos) const;

    /** @brief Write sample to delay line and advance */
    void writeSample(int tap, double sample);
};
