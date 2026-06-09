/**
 * @file Limiter10.h
 * @brief 砖墙限幅器(过采样插值真峰值检测+ISP/TPB合规计量) — Brick-Wall Limiter with True-Peak Detection via Oversampled Interpolation and ISP/TPB Compliance Metering
 *
 * 功能: 实现砖墙限幅器(Brick-wall limiter)，通过过采样插值(oversampled
 *       interpolation)进行真峰值检测(true-peak detection)，提供ISP/TPB
 *       (Inter-Sample Peak / True Peak Binding)合规计量。
 *
 * 协作: Compressor8(动态压缩) / Equalizer6(均衡器) / Dither5(抖动)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限幅器(过采样插值真峰值检测+ISP/TPB合规)
 */
class Limiter10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double truePeakDb = -120.0;
        double peakReductionDb = 0.0;
        double ispCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter10(QObject *parent = nullptr);
    ~Limiter10() override;

    /** @brief Set ceiling level in dB */
    void setCeiling(double ceilingDb);

    /** @brief Set release time in milliseconds */
    void setRelease(double releaseMs);

    /** @brief Set oversampling factor (1, 2, 4, 8) */
    void setOversampling(int factor);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Process audio through limiter */
    QVector<double> process(const QVector<double>& input);

    /** @brief Measure true peak of signal (interpolated) */
    double measureTruePeak(const QVector<double>& signal) const;

    /** @brief Get ISP/TPB compliance status */
    bool isCompliant() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double truePeakDb, double timeMs);

private:
    double m_ceilingDb = -1.0;
    double m_releaseMs = 50.0;
    int m_oversampling = 4;
    double m_sampleRate = 44100.0;

    double m_ceilingLinear = 0.0;
    double m_gain = 1.0;
    double m_gainRelease = 0.0;
    double m_truePeak = 0.0;
    int m_ispViolations = 0;

    // Sinc interpolation coefficients for 4x oversampling
    QVector<double> m_sincCoeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute sinc interpolation table */
    void buildSincTable();

    /** @brief Oversample a block using sinc interpolation */
    QVector<double> oversample(const QVector<double>& input) const;

    /** @brief Detect true peak in oversampled signal */
    double detectTruePeak(const QVector<double>& oversampled) const;

    /** @brief Compute gain reduction for given peak */
    double computeGainReduction(double peak) const;

    /** @brief Apply gain with smooth release envelope */
    double applyGain(double sample);
};
