/**
 * @file Chorus3.h
 * @brief 合唱效果器(调制多抽头延迟+立体声相位交错声像扩散) — Chorus Effect with Modulated Multitap Delay and Stereo Phase-Interleaved Voice Spreading
 *
 * 功能: 实现合唱效果器，支持调制多抽头延迟、
 *       立体声相位交错声像扩散和LFO波形选择。
 *
 * 协作: PhaseVocoder5(相位声码器) / BiquadFilter7(双二阶滤波器) / ReverbEffect4(混响效果)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 合唱效果器(调制多抽头延迟+立体声相位交错声像扩散)
 */
class Chorus3 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform type */
    enum LfoWaveform { Sine = 0, Triangle = 1, Sawtooth = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numTaps = 0;
        double baseDelayMs = 0.0;
        double depthMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus3(QObject *parent = nullptr);
    ~Chorus3() override;

    void setSampleRate(double rate);
    void setBaseDelayMs(double ms);
    void setModDepthMs(double ms);
    void setLfoRate(double hz);
    void setLfoWaveform(LfoWaveform wave);
    void setNumTaps(int taps);
    void setFeedback(double fb);
    void setMix(double wetDry);

    /** @brief Process mono input to stereo chorus output */
    QVector<QPair<double, double>> process(const QVector<double>& input);

    /** @brief Process single sample frame */
    QPair<double, double> processSample(double sample);

    /** @brief Reset delay lines */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_baseDelayMs = 7.0;
    double m_depthMs = 3.0;
    double m_lfoRate = 0.8;
    LfoWaveform m_waveform = Sine;
    int m_numTaps = 3;
    double m_feedback = 0.2;
    double m_mix = 0.5;

    QVector<double> m_delayLine;
    int m_writePos = 0;
    int m_delayLineSize = 0;

    double m_lfoPhase = 0.0;
    double m_phaseIncrement = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute LFO value at current phase */
    double lfoValue(double phase) const;

    /** @brief Read from delay line with fractional sample interpolation */
    double readDelay(int tap, double modOffset) const;

    /** @brief Advance LFO phase */
    void advanceLfo();

    /** @brief Update delay line size from parameters */
    void updateDelaySize();
};
