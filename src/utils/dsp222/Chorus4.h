/**
 * @file Chorus4.h
 * @brief 合唱效果器(调制延迟合奏+相位随机化LFO银行立体声扩展) — Chorus Effect with Ensemble of Modulated Delays and Stereo Spread via Phase-Randomized LFO Bank
 *
 * 功能: 实现合唱效果器，使用多个调制延迟构成合奏效果，
 *       通过相位随机化的LFO银行实现立体声扩展。
 *
 * 协作: FIRFilter7(FIR滤波器) / BiquadFilter6(双二阶滤波器) / Reverb5(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 合唱效果器(调制延迟+LFO银行立体声)
 */
class Chorus4 : public QObject {
    Q_OBJECT

public:
    /** @brief Chorus configuration */
    struct Config {
        int numVoices = 4;
        double baseDelayMs = 7.0;
        double depthMs = 3.0;
        double rateHz = 0.8;
        double feedback = 0.2;
        double mix = 0.5;
        double stereoSpread = 0.7;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        int numVoices = 0;
        int bufferSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus4(QObject *parent = nullptr);
    ~Chorus4() override;

    /** @brief Configure chorus parameters */
    void setConfig(const Config& config);

    /** @brief Prepare for processing at given sample rate and buffer size */
    void prepare(int sampleRate, int bufferSize);

    /** @brief Process mono input to stereo output [L,R,L,R,...] */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample, return stereo pair */
    void processSample(double input, double& outL, double& outR);

    /** @brief Reset delay lines and LFO phases */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    Config m_config;
    int m_sampleRate = 44100;

    // Per-voice delay lines
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_writePos;

    // Per-voice LFO state
    QVector<double> m_lfoPhase;
    QVector<double> m_lfoPhaseInc;
    QVector<double> m_lfoPhaseOffset;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize LFO bank with random phase offsets */
    void initLFOBank();

    /** @brief Advance LFO and return current mod value */
    double tickLFO(int voice);

    /** @brief Read from fractional delay position */
    double readDelay(int voice, double fraction) const;

    /** @brief Write to delay line */
    void writeDelay(int voice, double sample);
};
