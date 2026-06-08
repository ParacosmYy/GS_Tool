/**
 * @file Delay4.h
 * @brief 延迟线(节拍同步分数延迟插值+共振滤波反馈回路) — Delay Line with Tempo-Synced Fractional Delay Interpolation and Feedback Loop with Resonant Filter
 *
 * 功能: 实现延迟线效果器，支持节拍同步分数延迟插值、
 *       共振滤波反馈回路和多抽头延迟。
 *
 * 协作: BiquadFilter4(双二阶滤波) / Oscillator4(振荡器) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 延迟线(节拍同步分数延迟插值+共振滤波反馈回路)
 */
class Delay4 : public QObject {
    Q_OBJECT

public:
    /** @brief Tempo sync note values */
    enum TempoSync {
        Free = 0,       // Free delay time
        Whole = 1,      // Whole note
        Half = 2,       // Half note
        Quarter = 4,    // Quarter note
        Eighth = 8,     // Eighth note
        Sixteenth = 16  // Sixteenth note
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int bufferLength = 0;
        double delayTimeMs = 0.0;
        double feedback = 0.0;
        double mix = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay4(QObject *parent = nullptr);
    ~Delay4() override;

    /** @brief Initialize with sample rate and max delay */
    void init(double sampleRate, double maxDelayMs = 2000.0);

    /** @brief Set delay time in ms */
    void setDelayTime(double ms);

    /** @brief Set delay synced to tempo */
    void setDelaySync(TempoSync note, double bpm);

    /** @brief Set feedback amount [0..0.99] */
    void setFeedback(double fb);

    /** @brief Set wet/dry mix [0..1] */
    void setMix(double mix);

    /** @brief Set resonant filter cutoff frequency */
    void setFilterCutoff(double hz);

    /** @brief Set resonant filter resonance (Q) */
    void setFilterQ(double q);

    /** @brief Process a single sample */
    double processSample(double input);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Reset delay buffer */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void delayProcessed(int samples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_maxDelayMs = 2000.0;

    // Circular buffer
    QVector<double> m_buffer;
    int m_writeIdx = 0;
    double m_delaySamples = 0.0;

    double m_feedback = 0.4;
    double m_mix = 0.5;

    // Resonant filter state (1-pole lowpass in feedback)
    double m_filterCutoff = 5000.0;
    double m_filterQ = 0.707;
    double m_filterState = 0.0;
    double m_filterA = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
    quint64 m_sampleCount = 0;

    /** @brief Fractional delay with allpass interpolation */
    double readFractional(double delaySamples) const;

    /** @brief Apply resonant filter to sample */
    double applyFilter(double input);

    /** @brief Update filter coefficient */
    void updateFilterCoeff();
};
