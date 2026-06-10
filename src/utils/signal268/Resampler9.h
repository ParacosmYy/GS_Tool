/**
 * @file Resampler9.h
 * @brief 重采样器(多相分解与Kaiser窗sinc插值任意比率采样率转换) — Resampler with Polyphase Decomposition and Kaiser-Windowed Sinc Interpolation for Arbitrary Ratio Sample Rate Conversion
 *
 * 功能: 实现重采样器(Resampler)，采用多相分解(polyphase decomposition)和Kaiser窗sinc插值
 *       (Kaiser-windowed sinc interpolation)实现任意比率采样率转换。
 *
 * 协作: FirFilter6(FIR滤波器) / WaveletTransform5(小波变换) / WindowFunction4(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(多相分解与Kaiser窗sinc插值任意比率采样率转换)
 */
class Resampler9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRate = 0;
        int outputRate = 0;
        int inputSamples = 0;
        int outputSamples = 0;
        int filterTaps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler9(QObject *parent = nullptr);
    ~Resampler9() override;

    /** @brief Set input/output sample rates, filter taps, Kaiser beta */
    void setParameters(int inputRate, int outputRate, int numTaps = 64,
                       double kaiserBeta = 5.0);

    /** @brief Process input buffer, return resampled output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single input sample (streaming) */
    void processSample(double input, QVector<double>& outputSamples);

    /** @brief Get the polyphase filter bank */
    QVector<QVector<double>> filterBank() const;

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingDone(int inSamples, int outSamples, double timeMs);

private:
    int m_inputRate = 44100;
    int m_outputRate = 48000;
    int m_numTaps = 64;
    double m_kaiserBeta = 5.0;

    int m_L = 1;    // Interpolation factor
    int m_M = 1;    // Decimation factor
    int m_gcd = 1;

    QVector<double> m_filter;       // Prototype lowpass filter
    QVector<QVector<double>> m_polyphase; // Polyphase sub-filters
    QVector<double> m_delayLine;    // Input delay line for streaming
    int m_delayPos = 0;
    double m_phase = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Kaiser window value */
    double kaiserWindow(int n, int N, double beta) const;

    /** @brief Design prototype lowpass filter */
    void designFilter();

    /** @brief Decompose filter into polyphase sub-filters */
    void decomposePolyphase();

    /** @brief Compute GCD */
    static int gcd(int a, int b);
};
