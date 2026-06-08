/**
 * @file Expander7.h
 * @brief 下行扩展器(程序依赖释放+对数增益曲线自然门) — Downward Expander with Program-Dependent Release and Logarithmic Gain Curve for Natural Gate Behavior
 *
 * 功能: 实现下行扩展器(downward expander)动态处理器，采用程序依赖释放(program-dependent release)
 *       和对数增益曲线(logarithmic gain curve)，实现自然的噪声门行为。
 *
 * 协作: Compressor6(压缩器) / Limiter5(限制器) / DeEsser4(齿音消除)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 下行扩展器(程序依赖释放+对数增益曲线)
 */
class Expander7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        int numChannels = 0;
        double avgGainReduction = 0.0;
        double peakGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander7(QObject *parent = nullptr);
    ~Expander7() override;

    /** @brief Set expander parameters */
    void setParameters(double threshold, double ratio, double attack,
                       double release, double knee, double sampleRate = 44100.0);

    /** @brief Process a single sample frame (multi-channel) */
    QVector<double> processFrame(const QVector<double>& frame);

    /** @brief Process a block of interleaved samples */
    QVector<double> processBlock(const QVector<double>& data, int channels);

    /** @brief Get current gain level (linear, per channel) */
    QVector<double> currentGain() const;

    /** @brief Get gain reduction in dB (per channel) */
    QVector<double> gainReductionDb() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gateOpened(int channel);
    void gateClosed(int channel);
    void gainChanged(int channel, double gainDb);

private:
    double m_threshold = -40.0;    // dB
    double m_ratio = 4.0;         // expansion ratio
    double m_attack = 0.1;        // ms
    double m_release = 50.0;      // ms (base release)
    double m_knee = 6.0;          // dB (soft knee width)
    double m_sampleRate = 44100.0;
    double m_range = -80.0;       // dB (maximum attenuation)

    // Per-channel state
    QVector<double> m_gainEnvelope;    // current gain envelope (linear)
    QVector<double> m_gainReduction;   // current gain reduction (dB)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute logarithmic gain curve for given input level */
    double computeGainCurve(double inputDb) const;

    /** @brief Program-dependent release: faster for louder signals */
    double programRelease(double inputDb) const;

    /** @brief Convert dB to linear */
    static double dbToLinear(double db);

    /** @brief Convert linear to dB */
    static double linearToDb(double linear);

    /** @brief Smooth gain envelope with ballistics */
    double smoothEnvelope(double target, double current, double coeff) const;
};
