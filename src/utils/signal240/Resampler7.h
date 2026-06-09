/**
 * @file Resampler7.h
 * @brief 重采样器(多相分解+Farrow结构任意有理采样率转换) — Resampler with Polyphase Decomposition and Farrow Structure for Arbitrary Rational Sampling Rate Conversion
 *
 * 功能: 实现重采样器(Resampler)，采用多相分解(polyphase decomposition)将抗混叠滤波器分解
 *       为多个子滤波器实现高效内插，结合Farrow结构(Farrow structure)通过多项式近似实现
 *       任意有理采样率转换(arbitrary rational sampling rate conversion)。
 *
 * 协作: FirFilter3(FIR滤波器) / IirFilter4(IIR滤波器) / WindowSync5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(多相分解+Farrow结构任意有理采样率转换)
 */
class Resampler7 : public QObject {
    Q_OBJECT

public:
    /** @brief Resampler configuration */
    struct Config {
        int inputRate = 44100;
        int outputRate = 48000;
        int polyphaseBranches = 32;   // number of polyphase sub-filters
        int farrowOrder = 3;          // Farrow polynomial order
        double stopbandAtten = 80.0;  // stopband attenuation in dB
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRate = 0;
        int outputRate = 0;
        quint64 inputSamples = 0;
        quint64 outputSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler7(QObject *parent = nullptr);
    ~Resampler7() override;

    /** @brief Configure with input/output rates and parameters */
    bool configure(const Config& config);

    /** @brief Configure with rational ratio P/Q */
    bool configureRatio(int P, int Q, int branches = 32);

    /** @brief Process input samples, return resampled output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single input sample, returns 0+ output samples */
    QVector<double> processOne(double sample);

    /** @brief Flush remaining samples from polyphase buffer */
    QVector<double> flush();

    /** @brief Reset internal state (keep configuration) */
    void reset();

    /** @brief Get the exact rational conversion ratio */
    double ratio() const;

    /** @brief Get current configuration */
    const Config& config() const { return m_config; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingCompleted(quint64 inCount, quint64 outCount, double timeMs);

private:
    Config m_config;
    int m_P = 1;       // interpolation factor
    int m_Q = 1;       // decimation factor
    int m_filterLen = 0;

    QVector<QVector<double>> m_polyphase;  // [branch][tap] polyphase coefficients
    QVector<QVector<double>> m_farrowCoeff; // [order][tap] Farrow coefficients

    // State
    QVector<double> m_delayLine;   // circular delay buffer
    int m_delayPos = 0;
    double m_phaseAccum = 0.0;     // fractional phase accumulator

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design prototype lowpass filter */
    QVector<double> designPrototype(int len, double cutoff) const;

    /** @brief Decompose prototype into polyphase branches */
    void decomposePolyphase(const QVector<double>& proto, int branches);

    /** @brief Compute Farrow polynomial coefficients */
    void computeFarrowCoeffs(int order, int tapLen);

    /** @brief Evaluate polyphase branch at fractional phase */
    double evaluateBranch(int branch, double frac) const;

    /** @brief Greatest common divisor */
    static int gcd(int a, int b);
};
