/**
 * @file Flanger3.h
 * @brief 镶边效果器(可变深度梳状滤波+过零镶边与反相信号混合) — Flanger with Variable-Depth Comb Filtering and Through-Zero Flanging with Inverted Signal Blend
 *
 * 功能: 实现镶边效果器，支持可变深度梳状滤波、
 *       过零镶边和反相信号混合。
 *
 * 协作: Phaser2(移相器) / Delay4(延迟) / Chorus3(合唱)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(可变深度梳状滤波+过零镶边与反相信号混合)
 */
class Flanger3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int bufferSize = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief LFO waveform type */
    enum LfoType { Sine, Triangle, Sawtooth, Square };

    explicit Flanger3(QObject *parent = nullptr);
    ~Flanger3() override;

    void setSampleRate(int rate);
    void setBaseDelayMs(double ms);
    void setDepthMs(double ms);
    void setFeedback(double fb);
    void setMix(double mix);
    void setLfoRate(double hz);
    void setLfoType(LfoType type);
    void setInvertPhase(bool invert);

    /** @brief Initialize internal delay buffer */
    void initialize(int bufferSize);

    /** @brief Process a single sample */
    double processSample(double input);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Reset delay buffer and LFO phase */
    void reset();

    /** @brief Get current LFO value for visualization */
    double currentLfoValue() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_baseDelayMs = 1.0;     // Base delay in ms
    double m_depthMs = 2.0;         // Modulation depth in ms
    double m_feedback = 0.5;        // Feedback gain [0, 1)
    double m_mix = 0.5;             // Wet/dry mix
    double m_lfoRate = 0.5;         // LFO frequency in Hz
    LfoType m_lfoType = Sine;
    bool m_invertPhase = false;     // Through-zero inverted blend

    QVector<double> m_delayBuffer;
    int m_writeIdx = 0;
    int m_bufferSize = 0;
    double m_lfoPhase = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute LFO output for current phase */
    double computeLfo() const;

    /** @brief Fractional delay read with linear interpolation */
    double readDelay(double delaySamples) const;

    /** @brief Advance LFO phase by one sample */
    void advanceLfo();
};
