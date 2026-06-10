/**
 * @file Delay8.h
 * @brief 延迟效果(节拍同步调制与磁带回声Wow/Flutter模拟) — Delay with Tempo-Synced Modulation and Tape-Echo Wow/Flutter Simulation for Vintage Delay Effects
 *
 * 功能: 实现延迟效果(Delay effect)，采用节拍同步调制(tempo-synced modulation)与
 *       磁带回声Wow/Flutter模拟(tape-echo wow/flutter simulation)实现复古延迟效果。
 *
 * 协作: Chorus7(合唱) / Phaser6(移相器) / Reverb9(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 延迟效果(节拍同步调制与磁带回声Wow/Flutter模拟)
 */
class Delay8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int bufferSize = 0;
        int sampleRate = 0;
        double delayTimeMs = 0.0;
        double feedback = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tempo sync note values */
    enum SyncNote {
        FreeRunning = 0,
        WholeNote,
        HalfNote,
        QuarterNote,
        EighthNote,
        SixteenthNote,
        DottedQuarter,
        TripletEighth
    };

    explicit Delay8(QObject *parent = nullptr);
    ~Delay8() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(int rate);

    /** @brief Set delay time in milliseconds (free-running mode) */
    void setDelayTime(double ms);

    /** @brief Set tempo-synced delay via note value and BPM */
    void setTempoSync(SyncNote note, double bpm);

    /** @brief Set feedback amount (0.0..0.95) */
    void setFeedback(double fb);

    /** @brief Set mix level (0.0 = dry, 1.0 = wet) */
    void setMix(double mix);

    /** @brief Set wow depth (0.0..1.0) for tape modulation */
    void setWowDepth(double depth);

    /** @brief Set flutter rate in Hz */
    void setFlutterRate(double rate);

    /** @brief Process a block of mono samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset delay buffer */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double delayMs, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_delayMs = 250.0;
    double m_feedback = 0.4;
    double m_mix = 0.5;
    double m_wowDepth = 0.3;
    double m_flutterRate = 3.5;

    // Circular buffer
    QVector<double> m_buffer;
    int m_writePos = 0;
    int m_delaySamples = 0;

    // LFO state for wow/flutter
    double m_lfoPhase = 0.0;
    double m_lfoIncrement = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute tempo-synced delay in ms */
    static double syncToMs(SyncNote note, double bpm);

    /** @brief Update buffer size from current delay time */
    void updateBuffer();

    /** @brief Advance LFO and return modulated delay offset */
    double modulatedDelay() const;
};
