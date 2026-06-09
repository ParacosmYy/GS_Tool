/**
 * @file Flanger6.h
 * @brief 镶边效果器(双调制梳状滤波器+立体声自动声像) — Flanger with Dual Modulated Comb Filters and Stereo Auto-Pan for Wider Spatial Movement and Feedback Resonance
 *
 * 功能: 实现镶边效果器(Flanger)，使用双调制梳状滤波器(dual modulated
 *       comb filters)产生相位干涉，立体声自动声像(stereo auto-pan)
 *       实现更宽广的空间移动感，反馈共振(feedback resonance)增强效果深度。
 *
 * 协作: Phaser5(相位器) / Chorus3(合唱效果) / Delay2(延迟效果)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(双梳状滤波器+立体声自动声像)
 */
class Flanger6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        int numChannels = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger6(QObject *parent = nullptr);
    ~Flanger6() override;

    /** @brief Set sample rate and max delay in samples */
    void prepare(int sampleRate, int maxDelaySamples);

    /** @brief Set LFO rate (Hz), depth (0..1), feedback (0..0.95) */
    void setParams(double lfoRate, double depth, double feedback);

    /** @brief Set stereo auto-pan rate (Hz) and width (0..1) */
    void setStereoPan(double panRate, double panWidth);

    /** @brief Process interleaved stereo samples in-place */
    void process(QVector<double>& samples, int numFrames);

    /** @brief Process mono input, produce stereo interleaved output */
    QVector<double> processMono(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processCompleted(int numFrames, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_maxDelay = 2048;
    double m_lfoRate = 0.5;       // Hz
    double m_depth = 0.5;         // 0..1
    double m_feedback = 0.5;      // 0..0.95
    double m_panRate = 0.2;       // Hz
    double m_panWidth = 0.7;      // 0..1

    double m_lfoPhase1 = 0.0;    // LFO phase for comb filter 1
    double m_lfoPhase2 = 0.0;    // LFO phase for comb filter 2 (offset)
    double m_panPhase = 0.0;     // Auto-pan LFO phase

    // Circular delay buffers for each comb filter
    QVector<double> m_delayBuf1;
    QVector<double> m_delayBuf2;
    int m_writePos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute current LFO value (sinusoidal) */
    double lfoValue(double phase) const;

    /** @brief Read from delay buffer with fractional sample interpolation */
    double readFractional(const QVector<double>& buf, double delaySamples) const;

    /** @brief Advance LFO phase */
    void advancePhase(double& phase, double rate);
};
