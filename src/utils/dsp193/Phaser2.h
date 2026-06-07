/**
 * @file Phaser2.h
 * @brief 移相器(级联全通滤波+LFO扫频+立体声正交调制) — Phaser with Cascaded All-Pass Filter Stages, LFO Sweep Depth and Stereo Quadrature Modulation
 *
 * 功能: 实现移相器DSP效果，支持级联全通滤波器、
 *       LFO深度扫频控制和立体声正交调制输出。
 *
 * 协作: BiquadFilter6(双二阶) / Chorus3(合唱) / DelayLine4(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 移相器(级联全通+LFO扫频+立体声)
 */
class Phaser2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int numStages = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser2(QObject *parent = nullptr);
    ~Phaser2() override;

    void setSampleRate(double sr);
    void setStages(int stages);
    void setLfoFrequency(double freq);
    void setLfoDepth(double depth);
    void setFeedback(double fb);
    void setMix(double mix);

    /** @brief Process mono sample, return stereo output (L, R) */
    QPair<double, double> processSample(double input);

    /** @brief Process buffer in-place (mono to stereo interleaved) */
    QVector<double> processBuffer(const QVector<double>& input);

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_stages = 6;
    double m_lfoFreq = 0.5;
    double m_lfoDepth = 0.8;
    double m_feedback = 0.7;
    double m_mix = 0.5;

    // LFO state
    double m_lfoPhase = 0.0;
    double m_lfoPhaseInc = 0.0;

    // All-pass filter state per stage
    struct AllPassStage {
        double x1 = 0.0;  // previous input
        double y1 = 0.0;  // previous output
        double coeff = 0.0;
    };
    QVector<AllPassStage> m_allpass;

    // Feedback path
    double m_feedbackBuf = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update LFO phase increment */
    void updateLfoRate();

    /** @brief Advance LFO and return modulation value [0..1] */
    double advanceLfo();

    /** @brief Process one all-pass stage */
    double allpass(AllPassStage& stage, double input);
};
