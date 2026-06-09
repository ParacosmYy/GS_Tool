/**
 * @file Chorus6.h
 * @brief 合唱效果器(颤音调制变延迟+立体声交叉混响反馈) — Chorus Effect with Vibrato-Modulated Variable Delay and Stereo Cross-mixing with Feedback for Ensemble Thickness
 *
 * 功能: 实现合唱效果器(Chorus Effect)，使用颤音调制(vibrato modulation)
 *       驱动变延迟线(variable delay line)产生音高偏移，立体声交叉混音
 *       (stereo cross-mixing)配合反馈(feedback)增加合奏厚度感。
 *
 * 协作: Flanger5(镶边效果) / DelayEffect4(延迟效果) / Reverb3(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 合唱效果器(颤音变延迟+立体声交叉混音反馈)
 */
class Chorus6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamplesProcessed = 0;
        int sampleRate = 0;
        int numVoices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus6(QObject *parent = nullptr);
    ~Chorus6() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(int rate);

    /** @brief Set chorus parameters: base delay ms, depth ms, rate Hz, feedback, mix */
    void setParameters(double baseDelayMs, double depthMs,
                       double rateHz, double feedback, double mix);

    /** @brief Set number of chorus voices */
    void setNumVoices(int voices);

    /** @brief Process mono input, produce stereo output [L0,R0,L1,R1,...] */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset delay lines */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_numVoices = 3;
    double m_baseDelayMs = 7.0;
    double m_depthMs = 3.0;
    double m_rateHz = 0.8;
    double m_feedback = 0.3;
    double m_mix = 0.5;

    /** @brief Delay line with fractional read via linear interpolation */
    struct DelayLine {
        QVector<double> buffer;
        int writePos = 0;
        void init(int size);
        void write(double sample);
        double readFrac(double delaySamples) const;
        void advance();
    };

    QVector<DelayLine> m_delayLinesL;
    QVector<DelayLine> m_delayLinesR;
    QVector<double> m_lfoPhase;  // Per-voice LFO phase
    double m_crossL = 0.0;
    double m_crossR = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    static constexpr int MAX_DELAY_SAMPLES = 4096;

    /** @brief Initialize delay lines for current settings */
    void initDelayLines();

    /** @brief Compute LFO value for given voice and phase */
    double lfo(int voice) const;
};
