/**
 * @file Phaser4.h
 * @brief 相位器(12阶全通网络+随机LFO相位偏移合奏厚度调制) — Phaser with 12-Stage Allpass Network and Randomized LFO Phase Offset for Ensemble Thickness Modulation
 *
 * 功能: 实现12阶全通网络相位器，集成随机LFO相位偏移，
 *       产生合奏厚度调制效果，支持可变反馈深度。
 *
 * 协作: Chorus3(合唱效果) / DelayLine8(延迟线) / Reverb6(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相位器(12阶全通+随机LFO相位偏移)
 */
class Phaser4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numStages = 12;
        int blockSize = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser4(QObject *parent = nullptr);
    ~Phaser4() override;

    /** @brief Initialize phaser with sample rate and block size */
    bool prepare(double sampleRate, int blockSize = 256);

    /** @brief Process audio block, return modulated output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Set LFO rate in Hz */
    void setLfoRate(double rateHz);

    /** @brief Set LFO depth (0.0 - 1.0) */
    void setLfoDepth(double depth);

    /** @brief Set feedback gain (0.0 - 0.95) */
    void setFeedback(double feedback);

    /** @brief Set base frequency for allpass modulation range */
    void setBaseFrequency(double freqHz);

    /** @brief Set spread for randomized LFO phase offsets */
    void setEnsembleSpread(double spread);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    static constexpr int ALLPASS_STAGES = 12;

    double m_sampleRate = 44100.0;
    int m_blockSize = 256;
    double m_lfoRate = 0.5;
    double m_lfoDepth = 0.7;
    double m_feedback = 0.5;
    double m_baseFreq = 1000.0;
    double m_ensembleSpread = 0.3;

    // Per-stage LFO phase with randomized offsets
    QVector<double> m_lfoPhase;
    QVector<double> m_lfoPhaseOffset;

    // Per-stage allpass state
    QVector<double> m_allpassX;
    QVector<double> m_allpassY;

    // Feedback buffer
    double m_feedbackBuffer = 0.0;

    // Double precision phase accumulator
    double m_phaseAccum = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute allpass coefficient for given center frequency */
    double allpassCoefficient(double freqHz) const;

    /** @brief Process one sample through the allpass chain */
    double processSample(double input);

    /** @brief Advance LFO and return modulation value for stage */
    double lfoValueForStage(int stage);

    /** @brief Initialize random LFO phase offsets */
    void initPhaseOffsets();
};
