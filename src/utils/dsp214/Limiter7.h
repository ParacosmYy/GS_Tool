/**
 * @file Limiter7.h
 * @brief 砖墙限制器(前瞻缓冲+4x过采样真峰值检测) — Brick-Wall Limiter with Lookahead Buffer and True-Peak Detection via 4x Oversampled Inter-Sample Level Metering
 *
 * 功能: 实现砖墙限制器，支持前瞻延迟缓冲、4倍过采样
 *       真峰值检测和增益平滑衰减控制。
 *
 * 协作: EnvelopeDetector4(包络检测) / Compressor3(压缩器) / FIRFilter2(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限制器(前瞻缓冲+4x过采样真峰值)
 */
class Limiter7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        double ceiling = 0.0;
        double threshold = 0.0;
        double peakReduction = 0.0;
        int lookaheadSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter7(QObject *parent = nullptr);
    ~Limiter7() override;

    /** @brief Set ceiling level (dB) and lookahead time (ms) */
    void setParameters(double ceilingDb = -0.3, double lookaheadMs = 5.0,
                       double releaseMs = 50.0, double sampleRate = 44100.0);

    /** @brief Process single sample, returns limited output */
    double processOne(double input);

    /** @brief Process buffer of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current gain reduction in dB */
    double gainReductionDb() const;

    /** @brief Get true-peak level of last processed block */
    double truePeakLevel() const;

    /** @brief Measure true-peak via 4x oversampled interpolation */
    double measureTruePeak(double input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReduction, double timeMs);

private:
    double m_ceilingLinear = 0.966;   // -0.3 dB
    double m_thresholdLinear = 0.966;
    double m_releaseCoeff = 0.0;
    double m_sampleRate = 44100.0;
    int m_lookahead = 220;             // 5ms at 44100Hz

    double m_gain = 1.0;               // Current gain
    double m_peakReduction = 0.0;
    double m_truePeak = 0.0;

    // Lookahead delay buffer
    QVector<double> m_delayBuffer;
    int m_delayPos = 0;

    // 4x oversampling FIR coefficients (short sinc)
    QVector<double> m_osCoeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    // 4x oversampling state
    QVector<double> m_osHistory;

    /** @brief Build 4x oversampling FIR filter coefficients */
    void buildOversamplingFilter();

    /** @brief Compute gain reduction for a given peak level */
    double computeGain(double peakLevel) const;
};
