/**
 * @file Delay6.h
 * @brief 多抽头延迟(节拍同步抽头间距+反馈滤波与低/高切音色控制) — Multi-Tap Delay with Tempo-Synced Tap Spacing and Feedback Filtering with Low/High Cut Tone Control
 *
 * 功能: 实现多抽头延迟效果器(multi-tap delay)，支持节拍同步抽头间距(tempo-synced tap spacing)
 *       根据BPM自动计算延迟时间，通过反馈滤波(feedback filtering)配合低切/高切(low/high cut)
 *       音色控制塑造延迟尾音，实现专业音频延迟效果。
 *
 * 协作: BiquadFilter9(双二阶滤波) / Compressor7(动态压缩) / Reverb5(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多抽头延迟(节拍同步抽头间距+反馈滤波与低/高切音色控制)
 */
class Delay6 : public QObject {
    Q_OBJECT

public:
    /** @brief Single tap configuration */
    struct TapConfig {
        double beatFraction = 0.25;
        double gain = 0.6;
        double pan = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        double bpm = 120.0;
        int numTaps = 4;
        int framesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay6(QObject *parent = nullptr);
    ~Delay6() override;

    /** @brief Set sample rate */
    void setSampleRate(int sr);

    /** @brief Set tempo in BPM */
    void setBPM(double bpm);

    /** @brief Set tap configurations */
    void setTaps(const QVector<TapConfig>& taps);

    /** @brief Set feedback amount (0.0 - 0.95) */
    void setFeedback(double fb);

    /** @brief Set low-cut frequency for feedback filter */
    void setLowCut(double freq);

    /** @brief Set high-cut frequency for feedback filter */
    void setHighCut(double freq);

    /** @brief Process interleaved stereo samples in-place */
    void process(QVector<double>& samples, int numFrames);

    /** @brief Reset delay lines */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processCompleted(int frames, double timeMs);

private:
    double m_feedback = 0.4;
    double m_lowCut = 80.0;
    double m_highCut = 12000.0;
    double m_wetMix = 0.5;

    QVector<TapConfig> m_taps;
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_writePos;

    // Feedback biquad filter state
    double m_lcx[3] = {}, m_lcy[3] = {};
    double m_hcx[3] = {}, m_hcy[3] = {};
    double m_lcb[3] = {}, m_hcb[3] = {};

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Calculate delay samples for a beat fraction */
    int beatToSamples(double beatFraction) const;

    /** @brief Apply feedback tone filter */
    double applyFeedbackFilter(double sample);

    /** @brief Update biquad coefficients for feedback filter */
    void updateFilterCoeffs();
};
