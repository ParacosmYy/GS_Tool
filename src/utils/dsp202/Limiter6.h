/**
 * @file Limiter6.h
 * @brief 多级限幅器(ISP感知削波+软削波波形整形+真峰值限制) — Multi-Stage Limiter with ISP-Aware Clipping, Soft-Clip Waveshaper and True-Peak Ceiling
 *
 * 功能: 实现多级音频限幅器，支持ISP(帧间样本峰值)检测、
 *       软削波波形整形和真峰值限制。
 *
 * 协作: Compressor5(压缩器) / EqFilter7(均衡器) / MeterBridge4(电平表)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多级限幅器(ISP感知削波+软削波波形整形+真峰值限制)
 */
class Limiter6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        double peakInput = 0.0;
        double peakOutput = 0.0;
        double gainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter6(QObject *parent = nullptr);
    ~Limiter6() override;

    void setCeiling(double ceilingDb);
    void setThreshold(double thresholdDb);
    void setRelease(double releaseMs);
    void setOversampleRate(int rate);

    /** @brief Process interleaved stereo samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Compute true peak via oversampled interpolation */
    double truePeak(const QVector<double>& samples) const;

    /** @brief Apply soft-clip waveshaper (tanh-based) */
    double softClip(double sample) const;

    /** @brief Detect inter-sample peaks (ISP) via sinc interpolation */
    double detectISP(const QVector<double>& samples) const;

    /** @brief Compute gain reduction in dB */
    double gainReductionDb() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void limitingActive(double reductionDb, double peakDb);

private:
    double m_ceiling = -0.3;        // dBFS
    double m_threshold = -6.0;      // dBFS
    double m_release = 50.0;        // ms
    int m_oversampleRate = 4;

    double m_gain = 1.0;            // current gain reduction
    double m_sampleRate = 44100.0;

    // Oversampling filter coefficients
    QVector<double> m_firCoeffs;
    QVector<double> m_delayLine;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design FIR lowpass for oversampling */
    void designOversampleFilter();

    /** @brief Upsample a single sample */
    QVector<double> upsample(double sample);

    /** @brief Downsample back to original rate */
    double downsample(const QVector<double>& samples);

    /** @brief Envelope follower for gain computation */
    double envelope(double input, double state) const;
};
