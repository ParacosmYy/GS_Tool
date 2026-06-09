/**
 * @file Limiter9.h
 * @brief 砖墙限制器(前瞻缓冲+自动增益控制可配置攻击/释放包络) — Brick-Wall Limiter with Lookahead Buffer and Automatic Gain Control with Configurable Attack/Release Envelope
 *
 * 功能: 实现砖墙限制器(brick-wall limiter)，通过前瞻缓冲(lookahead buffer)
 *       预检测峰值并配合自动增益控制(automatic gain control)，使用可配置的
 *       攻击/释放包络(attack/release envelope)实现无失真峰值限制。
 *
 * 协作: EnvelopeDetector6(包络检测) / ZoomFFT5(缩放FFT) / BiquadFilter7(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限制器(前瞻缓冲+自动增益控制可配置攻击/释放包络)
 */
class Limiter9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int bufferSize = 0;
        double peakReductionDb = 0.0;
        double avgGainDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter9(QObject *parent = nullptr);
    ~Limiter9() override;

    /** @brief Set ceiling threshold in dB (e.g. 0.0) */
    void setCeiling(double db);

    /** @brief Set attack time in milliseconds */
    void setAttackTime(double ms);

    /** @brief Set release time in milliseconds */
    void setReleaseTime(double ms);

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set lookahead time in milliseconds */
    void setLookaheadTime(double ms);

    /** @brief Process input buffer, returns limited output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample through limiter */
    double processOne(double sample);

    /** @brief Reset internal buffers and gain */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double peakReductionDb, double timeMs);
    void clippingDetected(int sampleIndex, double inputLevel);

private:
    double m_ceilingDb = 0.0;
    double m_ceilingLin = 1.0;
    double m_attackMs = 5.0;
    double m_releaseMs = 50.0;
    double m_sampleRate = 44100.0;
    double m_lookaheadMs = 5.0;

    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;
    double m_gain = 1.0;

    QVector<double> m_lookaheadBuf;
    int m_lookaheadSize = 0;
    int m_bufPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_peakInputDb = -120.0;

    /** @brief Recalculate attack/release coefficients */
    void updateCoefficients();

    /** @brief Convert dB to linear */
    static double dbToLinear(double db);

    /** @brief Convert linear to dB */
    static double linearToDb(double lin);
};
