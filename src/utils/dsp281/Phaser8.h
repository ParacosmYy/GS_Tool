/**
 * @file Phaser8.h
 * @brief 相位器(多级全通反馈与包络跟踪调制深度的动态相位扫描效果) — Phaser with Multi-stage All-pass Feedback and Envelope-following Modulation Depth for Dynamic Phase Sweep Effects
 *
 * 功能: 实现相位器(phaser)，采用多级全通反馈(multi-stage all-pass feedback)
 *       与包络跟踪调制深度(envelope-following modulation depth)实现动态相位扫描效果(dynamic phase sweep effects)。
 *
 * 协作: Chorus7(合唱) / Delay6(延迟) / WaveGenerator5(波形发生器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相位器(多级全通反馈与包络跟踪调制深度)
 */
class Phaser8 : public QObject {
    Q_OBJECT

public:
    /** @brief Phaser parameters */
    struct Parameters {
        double rate = 0.5;         // LFO frequency (Hz)
        double depth = 0.7;        // Modulation depth (0..1)
        double feedback = 0.5;     // Feedback gain (0..0.95)
        double mix = 0.5;          // Dry/wet mix (0..1)
        int stages = 4;            // Number of all-pass stages (2..12)
        double baseFrequency = 1000.0;  // Base all-pass center frequency
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
        quint64 totalBlocks = 0;
    };

    explicit Phaser8(QObject *parent = nullptr);
    ~Phaser8() override;

    /** @brief Set phaser parameters */
    void setParameters(const Parameters& params);

    /** @brief Process a single sample */
    double processSample(double input);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Reset internal state */
    void reset();

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockDone(int samples, double peakEnvelope, double timeMs);

private:
    Parameters m_params;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;

    /** @brief All-pass filter state for each stage */
    QVector<double> m_xz;   // Previous input per stage
    QVector<double> m_yz;   // Previous output per stage

    /** @brief LFO phase accumulator */
    double m_lfoPhase = 0.0;

    /** @brief Envelope follower state */
    double m_envelope = 0.0;

    /** @brief Feedback accumulator */
    double m_feedbackSample = 0.0;

    /** @brief Compute LFO value (sine with phase accumulator) */
    double lfoValue();

    /** @brief Compute all-pass coefficient from center frequency */
    double allPassCoeff(double freq) const;

    /** @brief Process single all-pass stage */
    double allPassStage(double input, int stage, double coeff);

    /** @brief Update envelope follower */
    double followEnvelope(double sample);
};
