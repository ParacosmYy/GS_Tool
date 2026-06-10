/**
 * @file Flanger7.h
 * @brief 镶边效果器(立体声展宽调制梳状滤波器与反馈阻尼金属扫频效果) — Flanger with Stereo-Widened Modulated Comb Filter and Feedback Damping for Metallic Sweeping Effects
 *
 * 功能: 实现镶边效果器(Flanger)，采用立体声展宽调制梳状滤波器(stereo-widened modulated
 *       comb filter)和反馈阻尼(feedback damping)实现金属扫频效果。
 *
 * 协作: ChorusEffect5(合唱效果) / PhaserEffect5(移相效果) / DelayLine4(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(立体声展宽调制梳状滤波器与反馈阻尼金属扫频效果)
 */
class Flanger7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int bufferSize = 0;
        double sampleRate = 0.0;
        int numSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger7(QObject *parent = nullptr);
    ~Flanger7() override;

    /** @brief Set sample rate, delay range (ms), depth, rate (Hz), feedback, stereo width */
    void setParameters(double sampleRate, double minDelayMs, double maxDelayMs,
                       double depth, double rateHz, double feedback, double stereoWidth);

    /** @brief Process mono signal to stereo flanged output */
    QVector<QVector<double>> process(const QVector<double>& input);

    /** @brief Process single sample pair (inline) */
    void processSample(double input, double& outLeft, double& outRight);

    /** @brief Reset delay buffer state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numSamples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_minDelayMs = 0.5;
    double m_maxDelayMs = 5.0;
    double m_depth = 1.0;
    double m_rateHz = 0.5;
    double m_feedback = 0.7;
    double m_stereoWidth = 0.6;
    double m_damping = 0.85;

    int m_bufferSize = 0;
    QVector<double> m_delayBufferL;
    QVector<double> m_delayBufferR;
    int m_writePos = 0;
    double m_phase = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Advance LFO phase and return current modulation value */
    double lfoValue();

    /** @brief Read from circular buffer with fractional delay */
    double readBuffer(const QVector<double>& buffer, double delaySamples) const;
};
