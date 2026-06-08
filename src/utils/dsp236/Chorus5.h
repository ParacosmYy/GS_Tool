/**
 * @file Chorus5.h
 * @brief 合唱效果器(多声部调制延迟线+立体声相位偏移LFO) — Chorus Effect with Multi-Voice Modulated Delay Lines and Stereo Spread via Phase-Offset LFO Modulation
 *
 * 功能: 实现合唱效果器(Chorus effect)，采用多声部调制延迟线(multi-voice modulated delay lines)
 *       结合立体声相位偏移LFO调制(stereo spread via phase-offset LFO modulation)，
 *       产生丰富的合唱/合奏空间效果。
 *
 * 协作: ReverbFilter7(混响) / DelayFilter4(延迟) / Equalizer9(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 合唱效果器(多声部调制延迟线+立体声相位偏移LFO)
 */
class Chorus5 : public QObject {
    Q_OBJECT

public:
    /** @brief Single voice configuration */
    struct VoiceConfig {
        double delayMs = 7.0;       // base delay in ms
        double depth = 3.0;         // modulation depth in ms
        double rate = 0.8;          // LFO frequency in Hz
        double pan = 0.0;           // stereo pan (-1 left, +1 right)
        double mix = 0.5;           // wet/dry mix
        double phaseOffset = 0.0;   // LFO phase offset (0-2pi)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numVoices = 0;
        int sampleRate = 44100;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus5(QObject *parent = nullptr);
    ~Chorus5() override;

    /** @brief Set sample rate */
    void setSampleRate(int rate);

    /** @brief Set base delay in ms */
    void setBaseDelay(double ms);

    /** @brief Set LFO depth in ms */
    void setDepth(double ms);

    /** @brief Set LFO rate in Hz */
    void setRate(double hz);

    /** @brief Set stereo spread width (0.0 - 1.0) */
    void setStereoSpread(double spread);

    /** @brief Set feedback amount (0.0 - 0.95) */
    void setFeedback(double fb);

    /** @brief Set number of voices (1-8) */
    void setNumVoices(int voices);

    /** @brief Process mono input, return stereo output [left, right] */
    QVector<QVector<double>> process(const QVector<double>& input);

    /** @brief Process single sample, return stereo pair */
    QPair<double, double> processSample(double sample);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_baseDelay = 7.0;
    double m_depth = 3.0;
    double m_rate = 0.8;
    double m_spread = 0.6;
    double m_feedback = 0.2;
    int m_numVoices = 3;

    // Circular delay buffer per voice
    QVector<QVector<double>> m_delayBuffer;
    QVector<int> m_writePos;
    QVector<double> m_lfoPhase;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize delay buffers */
    void initBuffers();

    /** @brief Compute LFO value for a voice */
    double lfoValue(int voiceIdx) const;

    /** @brief Read from delay buffer with fractional index (linear interp) */
    double readDelay(int voiceIdx, double delaySamples) const;

    /** @brief Calculate delay buffer size from base delay + depth */
    int delayBufferSize() const;

    /** @brief Advance LFO phase */
    void advanceLFO();
};
