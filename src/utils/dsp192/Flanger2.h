/**
 * @file Flanger2.h
 * @brief 镶边效果器(调制梳状滤波器+立体声相位偏移+反馈共振控制) — Flanger with Modulated Comb Filter, Stereo Phase Offset and Feedback Resonance Control
 *
 * 功能: 实现镶边效果器，支持调制梳状滤波器、
 *       立体声相位偏移和反馈共振控制参数。
 *
 * 协作: ChorusEffect3(合唱) / PhaserEffect2(相位) / DelayLine4(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 镶边效果器(调制梳状+立体声偏移+反馈共振)
 */
class Flanger2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessed = 0;
        int sampleRate = 0;
        int bufferSize = 0;
        double depth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief LFO waveform type */
    enum LFOType { Sine, Triangle, Sawtooth, Square };

    explicit Flanger2(QObject *parent = nullptr);
    ~Flanger2() override;

    void setSampleRate(int sr);
    void setRate(double hz);
    void setDepth(double ms);
    void setFeedback(double fb);
    void setStereoPhase(double degrees);
    void setLFOType(LFOType type);

    /** @brief Process mono sample block, returns processed samples */
    QVector<double> processMono(const QVector<double>& input);

    /** @brief Process stereo sample block (interleaved L,R) */
    QVector<double> processStereo(const QVector<double>& interleaved);

    /** @brief Reset delay line buffers */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_rate = 0.5;        // LFO rate in Hz
    double m_depth = 3.0;       // Modulation depth in ms
    double m_feedback = 0.5;    // Feedback coefficient [0,1)
    double m_stereoPhase = 90.0;// Stereo phase offset in degrees
    LFOType m_lfoType = Sine;

    QVector<double> m_delayL;   // Circular buffer left
    QVector<double> m_delayR;   // Circular buffer right
    int m_writePos = 0;
    double m_phase = 0.0;       // Current LFO phase

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute LFO value at given phase */
    double lfoValue(double phase) const;

    /** @brief Read from delay buffer with fractional index (linear interp) */
    double readBuffer(const QVector<double>& buf, double index) const;

    /** @brief Advance LFO phase */
    void advancePhase();
};
