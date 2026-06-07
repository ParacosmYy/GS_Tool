/**
 * @file Phaser3.h
 * @brief 移相器(全通级联调制+立体声正交LFO空间扫描) — Phaser with Allpass Cascade Modulation and Stereo Quadrature Phase LFO for Spatial Sweep
 *
 * 功能: 实现移相器效果，支持全通级联调制、
 *       立体声正交相位LFO和空间扫描。
 *
 * 协作: Chorus2(合唱) / DelayLine5(延迟线) / BiquadFilter4(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 移相器(全通级联调制+立体声正交LFO空间扫描)
 */
class Phaser3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numStages = 0;
        double lfoFrequency = 0.0;
        double depth = 0.0;
        double feedback = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser3(int sampleRate = 44100, QObject *parent = nullptr);
    ~Phaser3() override;

    void setStages(int stages);
    void setLFOFrequency(double freq);
    void setDepth(double depth);
    void setFeedback(double fb);
    void setSampleRate(int sr);
    void setWetDryMix(double mix);

    /** @brief Process single stereo sample pair */
    void processSample(double inputL, double inputR,
                       double& outputL, double& outputR);

    /** @brief Process mono buffer */
    QVector<double> processMono(const QVector<double>& input);

    /** @brief Process stereo interleaved buffer */
    QVector<double> processStereo(const QVector<double>& interleaved);

    /** @brief Reset all internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    int m_sampleRate;
    int m_stages;
    double m_lfoFreq;
    double m_depth;
    double m_feedback;
    double m_wetDry;

    // Allpass state per stage (L/R)
    struct AllpassState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };

    QVector<AllpassState> m_apL;  // Left channel
    QVector<AllpassState> m_apR;  // Right channel

    double m_lfoPhaseL = 0.0;     // LFO phase (left)
    double m_lfoPhaseR = 0.0;     // LFO phase (right, quadrature)
    double m_fbL = 0.0, m_fbR = 0.0;  // Feedback accumulators

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute LFO value for given phase */
    double lfoValue(double phase) const;

    /** @brief Process single sample through allpass cascade */
    double processCascade(double input, QVector<AllpassState>& states,
                          double modAmount);

    /** @brief Second-order allpass filter */
    static double allpass2(double input, double coeff, AllpassState& state);
};
