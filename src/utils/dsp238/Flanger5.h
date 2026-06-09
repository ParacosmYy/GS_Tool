/**
 * @file Flanger5.h
 * @brief 镶边效果器(扫掠梳状滤波+再生反馈+可变深度立体声调制) — Flanger with Swept Comb Filter and Regenerative Feedback with Variable Depth Stereo Modulation
 *
 * 功能: 实现镶边效果器(Flanger effect)，采用扫掠梳状滤波器(swept comb filter)通过LFO调制
 *       延迟时间，结合再生反馈(regenerative feedback)控制谐振强度，支持可变深度立体声调制
 *       (variable depth stereo modulation)实现空间声场效果。
 *
 * 协作: Chorus6(合唱效果) / DelayLine3(延迟线) / Phaser4(相位器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(扫掠梳状滤波+再生反馈+可变深度立体声调制)
 */
class Flanger5 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform type */
    enum LfoWaveform { Sine = 0, Triangle = 1, Sawtooth = 2 };

    /** @brief Flanger parameters */
    struct Parameters {
        double rate = 0.5;          // LFO frequency in Hz
        double depth = 0.5;         // modulation depth 0-1
        double feedback = 0.7;      // regenerative feedback 0-0.95
        double mix = 0.5;           // wet/dry mix 0-1
        double delayBase = 0.002;   // base delay in seconds
        double delayRange = 0.005;  // sweep range in seconds
        LfoWaveform waveform = Sine;
        bool stereo = true;         // enable stereo modulation
        double stereoPhase = 90.0;  // stereo phase offset in degrees
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamplesProcessed = 0;
        int sampleRate = 44100;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger5(QObject *parent = nullptr);
    ~Flanger5() override;

    /** @brief Initialize with sample rate */
    bool init(int sampleRate);

    /** @brief Set flanger parameters */
    void setParameters(const Parameters& params);

    /** @brief Process mono audio buffer in-place */
    void processMono(QVector<double>& buffer);

    /** @brief Process stereo interleaved buffer in-place */
    void processStereo(QVector<double>& buffer);

    /** @brief Process single stereo sample pair */
    void processSample(double& left, double& right);

    /** @brief Reset delay lines and LFO phase */
    void reset();

    const Parameters& parameters() const { return m_params; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    Parameters m_params;
    int m_sampleRate = 44100;
    int m_delayLineSize = 0;
    int m_writePos = 0;

    QVector<double> m_delayLeft;    // circular delay buffer L
    QVector<double> m_delayRight;   // circular delay buffer R
    double m_lfoPhase = 0.0;        // current LFO phase
    double m_lfoPhaseInc = 0.0;     // LFO phase increment per sample

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute LFO value for given phase */
    double lfoValue(double phase) const;

    /** @brief Read from circular delay buffer with fractional index */
    double readFractional(const QVector<double>& buf, double index) const;
};
