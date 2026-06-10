/**
 * @file Limiter12.h
 * @brief 限幅器(多频段前瞻与过采样帧间真峰值检测的广播合规处理) — Limiter with Multi-band Lookahead and Inter-sample True Peak Detection via Oversampling for Broadcast Compliance
 *
 * 功能: 实现限幅器(limiter)，采用多频段前瞻(multi-band lookahead)
 *       与过采样帧间真峰值检测(inter-sample true peak detection via oversampling)实现广播合规处理(broadcast compliance)。
 *
 * 协作: Compressor10(压缩器) / FIR8(FIR滤波器) / EnvelopeDetector9(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 限幅器(多频段前瞻与过采样帧间真峰值检测)
 */
class Limiter12 : public QObject {
    Q_OBJECT

public:
    /** @brief Limiter band configuration */
    struct BandConfig {
        double lowFreq = 200.0;
        double highFreq = 2000.0;
        double threshold = -1.0;    // dBTP
        double ceiling = -0.5;      // dBTP
    };

    /** @brief Limiter configuration */
    struct LimiterConfig {
        int numBands = 4;
        int lookaheadSamples = 64;
        int oversampleFactor = 4;
        double threshold = -1.0;
        double ceiling = -0.5;
        double release = 50.0;      // ms
    };

    /** @brief Processing result */
    struct LimiterResult {
        QVector<double> output;
        double peakIn = 0.0;
        double peakOut = 0.0;
        double gainReduction = 0.0;
        bool clippingOccurred = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter12(QObject *parent = nullptr);
    ~Limiter12() override;

    void setConfig(const LimiterConfig& cfg);

    /** @brief Process audio frame */
    LimiterResult process(const QVector<double>& input);

    /** @brief True peak detection via 4x oversampling */
    double truePeak(const QVector<double>& frame) const;

    /** @brief Compute gain reduction for a peak value */
    double computeGain(double peak) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int n, double peakOut, double gr, double timeMs);

private:
    LimiterConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_delayLine;
    QVector<QVector<double>> m_bandBuffers;
    QVector<double> m_smoother;
    QVector<double> m_osFilter;      // Oversampling LPF coefficients

    int m_delayPos = 0;
    double m_currentGain = 1.0;

    /** @brief Design sinc interpolation filter for oversampling */
    void designOversampleFilter();

    /** @brief Oversample a single sample */
    QVector<double> oversample(double sample) const;

    /** @brief Multi-band crossover filter */
    void crossover(const QVector<double>& input);

    /** @brief Smooth gain with ballistics */
    double smoothGain(double target);
};
