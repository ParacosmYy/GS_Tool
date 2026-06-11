/**
 * @file Phaser9.h
 * @brief 移相器(级联全通网络与LFO调制陷波扫频实现多级相位抵消扫频效果) — Phaser with Cascaded All-pass Network and LFO-modulated Notch Sweep for Multi-stage Phase Cancellation Sweep Effect
 *
 * 功能: 实现移相器(phaser)，采用级联全通网络(cascaded all-pass network)
 *       与LFO调制陷波扫频(LFO-modulated notch sweep)实现多级相位抵消扫频效果(multi-stage phase cancellation sweep effect)。
 *
 * 协作: Flanger8(镶边器) / Chorus7(合唱器) / DelayEffect6(延迟效果)
 */
#pragma once

#include <QObject>
#include <QVector>

class Phaser9 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform type */
    enum class LFOType {
        Sine = 0,
        Triangle = 1,
        Square = 2
    };

    /** @brief Phaser processing result */
    struct ProcessResult {
        QVector<double> output;
        double peakLevel = 0.0;
        double rmsLevel = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser9(QObject *parent = nullptr);
    ~Phaser9() override;

    void setRate(double Hz);
    void setDepth(double depth);
    void setStages(int stages);
    void setFeedback(double fb);
    void setMix(double mix);
    void setLFOType(LFOType type);

    /** @brief Process a frame of audio samples */
    ProcessResult process(const QVector<double>& input);

    /** @brief Reset internal state (clear delay buffers) */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int samples, double peak, double timeMs);

private:
    double m_rate = 0.5;        // LFO rate in Hz
    double m_depth = 0.7;       // Modulation depth
    int m_stages = 4;           // Number of all-pass stages
    double m_feedback = 0.5;    // Feedback amount
    double m_mix = 0.5;         // Wet/dry mix
    LFOType m_lfoType = LFOType::Sine;
    double m_sampleRate = 44100.0;

    // Internal state
    double m_lfoPhase = 0.0;
    double m_feedbackSample = 0.0;
    QVector<double> m_allPassHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute current LFO value */
    double computeLFO() const;

    /** @brief Single all-pass filter stage */
    double allPassStage(double input, double coeff, double& history) const;

    /** @brief Compute all-pass coefficient from frequency */
    double freqToAllPassCoeff(double freq) const;
};
