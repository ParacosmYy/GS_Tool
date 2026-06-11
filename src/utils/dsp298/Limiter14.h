/**
 * @file Limiter14.h
 * @brief 真峰值限制器(过采样样本间峰值检测与前瞻增益平滑实现广播合规响度归一化) — True Peak Limiter with Oversampled Inter-sample Detection and Lookahead Gain Smoothing for Broadcast-compliant Loudness Normalization
 *
 * 功能: 实现真峰值限制器(true peak limiter)，采用过采样样本间峰值检测(oversampled inter-sample detection)
 *       与前瞻增益平滑(lookahead gain smoothing)实现广播合规响度归一化(broadcast-compliant loudness normalization)。
 *
 * 协作: Compressor10(动态压缩) / Equalizer(均衡器) / LoudnessMeter(响度计)
 */
#pragma once

#include <QObject>
#include <QVector>

class Limiter14 : public QObject {
    Q_OBJECT

public:
    /** @brief Limiter configuration */
    struct LimiterConfig {
        double thresholdDb = -1.0;      // ceiling in dB
        double attackMs = 1.0;
        double releaseMs = 50.0;
        double lookaheadMs = 5.0;
        int oversampleFactor = 4;
        double sampleRate = 44100.0;
    };

    /** @brief Processing result */
    struct LimiterResult {
        QVector<double> output;
        double peakInputDb = -120.0;
        double peakOutputDb = -120.0;
        double gainReductionDb = 0.0;
        int numSamples = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalProcesses = 0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter14(QObject *parent = nullptr);
    ~Limiter14() override;

    void setConfig(const LimiterConfig& config);

    /** @brief Process input buffer through the true peak limiter */
    LimiterResult process(const QVector<double>& input);

    /** @brief Reset internal state (delay lines, envelope) */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int n, double gainReduction, double timeMs);

private:
    LimiterConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_gainReductionSum = 0.0;

    // Internal state
    QVector<double> m_delayLine;
    QVector<double> m_oversampleBuffer;
    int m_delayWritePos = 0;
    double m_envelope = 1.0;    // current gain envelope (linear)
    double m_thresholdLin = 1.0;

    /** @brief Upsample input by factor N using sinc interpolation */
    QVector<double> upsample(const QVector<double>& input, int factor) const;

    /** @brief Downsample back to original rate */
    QVector<double> downsample(const QVector<double>& upsampled, int factor) const;

    /** @brief Detect true peak (inter-sample peak via oversampling) */
    double detectTruePeak(const QVector<double>& upsampled) const;

    /** @brief Compute gain reduction for detected peak */
    double computeGainReduction(double peakLin) const;

    /** @brief Smooth gain with attack/release envelope */
    void smoothEnvelope(double targetGain, int numSamples);
};
