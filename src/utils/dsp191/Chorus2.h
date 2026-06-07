/**
 * @file Chorus2.h
 * @brief 合唱效果器(多声部LFO调制+立体声展宽+颤音深度控制) — Chorus Effect with Multi-Voice LFO Modulation, Stereo Spread and Vibrato Depth Control
 *
 * 功能: 实现合唱效果器，支持多声部LFO调制、立体声展宽、
 *       颤音深度控制和可配置延迟/反馈参数。
 *
 * 协作: FirFilter12(FIR滤波) / IirFilter13(IIR滤波) / Oscillator6(振荡器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 合唱效果器(多声部LFO+立体声展宽+颤音控制)
 */
class Chorus2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int numVoices = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single voice configuration */
    struct VoiceConfig {
        double delayMs = 7.0;
        double depth = 1.0;
        double rate = 0.5;
        double phase = 0.0;
        double pan = 0.0;
    };

    explicit Chorus2(QObject *parent = nullptr);
    ~Chorus2() override;

    void setSampleRate(int sr);
    void setBaseDelay(double ms);
    void setVibratoDepth(double depth);
    void setStereoSpread(double spread);
    void setFeedback(double fb);
    void setMix(double wet);
    void setVoices(const QVector<VoiceConfig>& voices);

    /** @brief Process mono input, produce stereo output */
    QPair<QVector<double>, QVector<double>> process(
        const QVector<double>& input);

    /** @brief Process mono input, produce mono output */
    QVector<double> processMono(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, int voices, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_baseDelay = 7.0;
    double m_vibratoDepth = 1.0;
    double m_stereoSpread = 0.5;
    double m_feedback = 0.3;
    double m_wet = 0.5;

    QVector<VoiceConfig> m_voices;
    QVector<double> m_delayBuffer;
    int m_writePos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LFO output: sine wave at given phase and rate */
    double lfo(double phase, double rate, int sample) const;

    /** @brief Fractional delay read from buffer */
    double fractionalRead(double delaySamples) const;

    /** @brief Initialize default voices */
    void initDefaultVoices();
};
