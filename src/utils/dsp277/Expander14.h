/**
 * @file Expander14.h
 * @brief 动态扩展器(并行压缩下行混合与RMS电平检测的透明向上动态扩展) — Expander with Parallel Compression Downward Blend and RMS-level Detection for Transparent Upward Dynamic Expansion
 *
 * 功能: 实现动态扩展器(Expander)，采用并行压缩下行混合(parallel compression downward blend)
 *       与RMS电平检测(RMS-level detection)实现透明向上动态扩展(transparent upward dynamic expansion)。
 *
 * 协作: Compressor12(压缩器) / Limiter10(限幅器) / Gate9(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态扩展器(并行压缩下行混合与RMS电平检测)
 */
class Expander14 : public QObject {
    Q_OBJECT

public:
    /** @brief Expander parameters */
    struct Params {
        double threshold = -20.0;    // Threshold in dB
        double ratio = 2.0;         // Expansion ratio
        double attack = 5.0;        // Attack time in ms
        double release = 50.0;      // Release time in ms
        double knee = 6.0;          // Soft knee width in dB
        double range = 40.0;        // Max expansion in dB
        double parallelMix = 0.3;   // Parallel compression blend (0-1)
        double makeupGain = 0.0;    // Makeup gain in dB
    };

    /** @brief Frame-level analysis result */
    struct FrameAnalysis {
        double inputDb = 0.0;
        double gainDb = 0.0;
        double outputDb = 0.0;
        double rmsDb = 0.0;
        double envelope = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        double avgInputDb = 0.0;
        double avgGainDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander14(QObject *parent = nullptr);
    ~Expander14() override;

    /** @brief Set expander parameters */
    void setParams(const Params& params);

    /** @brief Process a single sample */
    double processSample(double sample);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Analyze frame without processing */
    FrameAnalysis analyzeFrame(const QVector<double>& frame) const;

    /** @brief Reset internal state */
    void reset();

    const Params& params() const { return m_params; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gainChanged(double gainDb, double inputDb, double timeMs);
    void blockProcessed(int frames, double avgGain, double timeMs);

private:
    Params m_params;
    Stats m_stats;
    double m_timeSum = 0.0;

    double m_envelope = 0.0;     // Current envelope follower value
    double m_sampleRate = 44100.0;

    /** @brief Convert dB to linear */
    static double dbToLinear(double db);

    /** @brief Convert linear to dB */
    static double linearToDb(double linear);

    /** @brief Compute RMS level of a frame */
    double computeRms(const QVector<double>& frame) const;

    /** @brief Compute expansion gain for a given input level */
    double computeGain(double inputDb) const;

    /** @brief Smooth gain transition with ballistics */
    double applyBallistics(double targetGain, double sampleRate);
};
