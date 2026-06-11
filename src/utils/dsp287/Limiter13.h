/**
 * @file Limiter13.h
 * @brief 砖墙限制器(前视延迟与自适应释放零过冲峰值控制) — Brick-wall Limiter with Look-ahead Delay and Adaptive Release for Zero-overshoot Peak Control
 *
 * 功能: 实现砖墙限制器(brick-wall limiter)，采用前视延迟(look-ahead delay)
 *       与自适应释放增益衰减(adaptive release gain reduction)实现零过冲峰值控制(zero-overshoot peak control)。
 *
 * 协作: Compressor12(压缩器) / Expander8(扩展器) / EnvelopeDetector10(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限制器(前视延迟与自适应释放零过冲峰值控制)
 */
class Limiter13 : public QObject {
    Q_OBJECT

public:
    /** @brief Limiter parameters */
    struct LimiterParams {
        double threshold = -3.0;        // dB
        double ceiling = -0.3;          // dB
        double attack = 0.1;            // ms
        double release = 50.0;          // ms
        double sampleRate = 44100.0;    // Hz
        int lookAhead = 512;            // samples
    };

    /** @brief Processing statistics */
    struct Stats {
        quint64 totalSamples = 0;
        double peakInputDb = -120.0;
        double peakOutputDb = -120.0;
        double maxGainReduction = 0.0;  // dB (negative)
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter13(QObject *parent = nullptr);
    ~Limiter13() override;

    void setParams(const LimiterParams& params);

    /** @brief Process a block of samples (mono) */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process interleaved multi-channel audio */
    QVector<QVector<double>> processMultiChannel(const QVector<QVector<double>>& input);

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int samples, double peakReduction, double timeMs);

private:
    LimiterParams m_params;
    Stats m_stats;
    double m_timeSum = 0.0;

    double m_gainComputerSmooth = 1.0;
    int m_lookAheadWritePos = 0;

    QVector<double> m_delayLine;
    QVector<double> m_gainReductionHistory;

    // Envelope follower state
    double m_envelope = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    /** @brief Compute gain reduction for a sample */
    double computeGainReduction(double inputDb);

    /** @brief Smooth gain reduction with adaptive release */
    double smoothGainReduction(double targetReduction);

    /** @brief Convert linear amplitude to dB */
    static double ampToDb(double amp);

    /** @brief Convert dB to linear amplitude */
    static double dbToAmp(double db);

    /** @brief Initialize coefficients from sample rate and time constants */
    void updateCoefficients();

    /** @brief Advance delay line by one sample, return delayed sample */
    double advanceDelayLine(double sample);
};
