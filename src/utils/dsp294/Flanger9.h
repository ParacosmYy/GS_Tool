/**
 * @file Flanger9.h
 * @brief 镶边效果器(过零延迟与调制全通插值实现金属扫频梳状滤波反馈效果) — Flanger with Through-zero Delay and Modulated All-pass Interpolation for Metallic Swept Comb Filter Feedback Effect
 *
 * 功能: 实现镶边效果器(Flanger)，采用过零延迟(through-zero delay)
 *       与调制全通插值(modulated all-pass interpolation)实现金属扫频梳状滤波反馈效果(metallic swept comb filter feedback effect)。
 *
 * 协作: ChorusEffect8(合唱效果) / PhaserEffect7(移相效果) / DelayLine6(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>

class Flanger9 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform type */
    enum LfoWaveform { Sine, Triangle, Sawtooth, Square };

    /** @brief Flanger parameters */
    struct Parameters {
        double rate = 0.5;           // LFO frequency in Hz
        double depth = 0.003;        // Modulation depth in seconds
        double feedback = 0.5;       // Feedback gain [0, 0.99]
        double mix = 0.5;            // Dry/wet mix [0, 1]
        double baseDelay = 0.002;    // Base delay in seconds
        LfoWaveform waveform = Sine;
        bool stereoPhase = false;    // 180° phase offset for stereo
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int blockSize = 0;
        double sampleRate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger9(QObject *parent = nullptr);
    ~Flanger9() override;

    void setSampleRate(double rate);
    void setParameters(const Parameters& params);

    /** @brief Process a block of mono samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process a block of stereo samples (interleaved L,R) */
    QVector<double> processStereo(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void resetDelayLine();

signals:
    void processDone(int numSamples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    Parameters m_params;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Circular delay buffer for through-zero flanging
    QVector<double> m_delayBuffer;
    int m_delayWritePos = 0;
    int m_delayLength = 0;

    // Feedback state
    double m_feedbackSample = 0.0;

    // LFO phase accumulator
    double m_lfoPhase = 0.0;

    /** @brief Advance LFO and return current modulation value [-1, 1] */
    double tickLfo();

    /** @brief Read from delay buffer with all-pass interpolation */
    double readDelayAllpass(double delaySamples) const;

    /** @brief Read from delay buffer for negative (through-zero) delays */
    double readThroughZero(double delaySamples, double currentInput) const;

    /** @brief Get LFO value for given waveform and phase */
    double lfoValue(LfoWaveform wave, double phase) const;
};
