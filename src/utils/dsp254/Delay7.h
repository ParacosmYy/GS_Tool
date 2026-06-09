/**
 * @file Delay7.h
 * @brief 延迟效果(节拍同步多抽头+乒乓立体声路由+每抽头滤波反馈控制) — Delay with Tempo-Synced Multi-Tap and Ping-Pong Stereo Routing with Per-Tap Filter and Feedback Control
 *
 * 功能: 实现延迟效果处理器(delay effect)，支持节拍同步多抽头(tempo-synced
 *       multi-tap)、乒乓立体声路由(ping-pong stereo routing)交替左右声道
 *       输出，每抽头低通滤波器(per-tap filter)和反馈控制(feedback control)。
 *
 * 协作: Reverb6(混响) / Chorus4(合唱) / Equalizer3(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 延迟效果(节拍同步多抽头+乒乓立体声)
 */
class Delay7 : public QObject {
    Q_OBJECT

public:
    /** @brief Configuration for a single delay tap */
    struct TapConfig {
        double delayBeats = 0.25;     // Delay time in beats
        double feedback = 0.4;        // Feedback gain [0,1)
        double filterCutoff = 0.8;    // Low-pass cutoff normalized [0,1]
        double pan = 0.0;             // Pan: -1(left) to +1(right)
        double gain = 0.7;            // Tap output gain
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        double tempoBpm = 120.0;
        int numTaps = 0;
        int numFramesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay7(int sampleRate = 44100, QObject *parent = nullptr);
    ~Delay7() override;

    /** @brief Set tempo in BPM for beat-synced delay */
    void setTempo(double bpm);

    /** @brief Set delay tap configurations */
    void setTaps(const QVector<TapConfig>& taps);

    /** @brief Enable/disable ping-pong stereo routing */
    void setPingPong(bool enabled);

    /** @brief Process a block of mono samples -> stereo output */
    QVector<QVector<double>> process(const QVector<double>& input);

    /** @brief Process single sample -> stereo [left, right] */
    QVector<double> processSample(double sample);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double timeMs);

private:
    int m_sampleRate;
    double m_tempoBpm = 120.0;
    bool m_pingPong = false;
    QVector<TapConfig> m_taps;

    // Per-tap delay buffer and filter state
    QVector<QVector<double>> m_delayBuffers;
    QVector<int> m_writePos;
    QVector<double> m_filterState;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Convert beat fraction to sample count */
    int beatsToSamples(double beats) const;

    /** @brief Apply one-pole low-pass filter */
    double applyFilter(double input, double cutoff, double& state) const;
};
