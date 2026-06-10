/**
 * @file Limiter11.h
 * @brief 限幅器(前瞻缓冲与真峰值采样点间插值广播安全电平控制) — Limiter with Lookahead Buffer and True-Peak Detection via Inter-Sample Interpolation for Broadcast-Safe Level Control
 *
 * 功能: 实现限幅器(Limiter)，采用前瞻缓冲(lookahead buffer)与真峰值检测(true-peak detection)
 *       采样点间插值(inter-sample interpolation)实现广播安全电平控制(broadcast-safe level control)。
 *
 * 协作: Compressor10(压缩器) / EnvelopeDetector8(包络检测器) / DeEsser6(嘶声消除器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 限幅器(前瞻缓冲与真峰值采样点间插值广播安全电平控制)
 */
class Limiter11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double ceiling = 0.0;
        double truePeakDb = -120.0;
        double gainReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter11(QObject *parent = nullptr);
    ~Limiter11() override;

    /** @brief Set output ceiling in dB (e.g. -1.0 dBTP) */
    void setCeiling(double ceilingDb);

    /** @brief Set lookahead time in milliseconds */
    void setLookaheadMs(double ms);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set release time in milliseconds */
    void setReleaseMs(double ms);

    /** @brief Process input samples through limiter */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get true-peak level in dBTP */
    double truePeakDb() const;

    /** @brief Get max gain reduction applied in dB */
    double maxGainReductionDb() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void limitingApplied(double gainReduction, double truePeak, double timeMs);

private:
    double m_ceilingDb = -1.0;
    double m_lookaheadMs = 5.0;
    double m_sampleRate = 44100.0;
    double m_releaseMs = 50.0;

    QVector<double> m_lookaheadBuf;
    int m_lookaheadSize = 0;
    int m_bufPos = 0;

    double m_gain = 1.0;        // current gain (linear)
    double m_truePeakDb = -120.0;
    double m_maxGainReductionDb = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize lookahead buffer */
    void initBuffer();

    /** @brief Estimate true-peak via 4x oversampled sinc interpolation */
    double estimateTruePeak(double x0, double xm1, double xp1, double xp2) const;

    /** @brief Compute gain reduction for a peak sample */
    double computeGain(double peakLinear) const;
};
