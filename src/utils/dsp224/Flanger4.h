/**
 * @file Flanger4.h
 * @brief 镶边效果器(双调制延迟线+立体声交叉反馈过零镶边) — Flanger with Dual Modulated Delay Lines and Stereo-Crossed Feedback for Through-Zero Flanging Effect
 *
 * 功能: 实现双延迟线镶边效果器，LFO调制延迟时间，
 *       立体声交叉反馈实现过零(through-zero)镶边效果。
 *
 * 协作: Chorus3(合唱) / Phaser2(移相器) / Delay5(延迟)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(双延迟线+立体声交叉反馈)
 */
class Flanger4 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform type */
    enum Waveform { Sine, Triangle, Sawtooth, Square };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        int blockSize = 0;
        int totalSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger4(QObject *parent = nullptr);
    ~Flanger4() override;

    /** @brief Configure flanger: sampleRate, delay(ms), depth(ms), rate(Hz), feedback, mix */
    void setParameters(int sampleRate = 44100, double baseDelayMs = 1.0,
                       double depthMs = 4.0, double rateHz = 0.5,
                       double feedback = 0.7, double mix = 0.5,
                       Waveform wave = Triangle);

    /** @brief Process stereo sample pair (inL, inR) -> (outL, outR) */
    void processSample(double inL, double inR, double* outL, double* outR);

    /** @brief Process block of interleaved stereo samples [L,R,L,R,...] */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Reset delay lines and LFO phase */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_baseDelayMs = 1.0;
    double m_depthMs = 4.0;
    double m_rateHz = 0.5;
    double m_feedback = 0.7;
    double m_mix = 0.5;
    Waveform m_waveform = Triangle;

    // Dual delay lines (stereo)
    QVector<double> m_delayLineL;
    QVector<double> m_delayLineR;
    int m_writePos = 0;
    int m_delayLength = 0;

    // LFO state
    double m_lfoPhase = 0.0;
    double m_lfoInc = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Advance LFO and return current value [-1, 1] */
    double tickLFO();

    /** @brief Read from delay line with fractional position */
    double readFractional(const QVector<double>& line, double position) const;
};
