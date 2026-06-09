/**
 * @file EnvelopeDetector6.h
 * @brief 包络检测器(Hilbert变换解析信号+峰值保持衰减可配置时间常数) — Envelope Detector with Analytic Signal via Hilbert Transform and Peak-Hold Decay with Configurable Time Constant
 *
 * 功能: 实现包络检测器(envelope detector)，通过Hilbert变换(Hilbert transform)
 *       构造解析信号(analytic signal)获取瞬时幅度，结合峰值保持衰减(peak-hold
 *       decay)机制使用可配置时间常数(configurable time constant)实现平滑包络提取。
 *
 * 协作: Limiter9(砖墙限制器) / ZoomFFT5(缩放FFT) / BiquadFilter7(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(Hilbert变换解析信号+峰值保持衰减可配置时间常数)
 */
class EnvelopeDetector6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double peakEnvelope = 0.0;
        double avgEnvelope = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector6(QObject *parent = nullptr);
    ~EnvelopeDetector6() override;

    /** @brief Set decay time constant in milliseconds */
    void setDecayTimeConstant(double ms);

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set FFT size for Hilbert transform (power of 2) */
    void setFFTSize(int n);

    /** @brief Set mode: 0=Hilbert analytic, 1=peak-hold decay */
    void setMode(int mode);

    /** @brief Process input signal, returns envelope */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get instantaneous phase from analytic signal */
    QVector<double> instantaneousPhase() const;

    /** @brief Get instantaneous frequency */
    QVector<double> instantaneousFrequency() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double peakEnv, double timeMs);

private:
    double m_decayMs = 100.0;
    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;
    int m_mode = 0;  // 0=Hilbert, 1=peak-hold

    QVector<double> m_lastPhase;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hilbert transform via FFT: returns analytic signal real, imag */
    void hilbertTransform(const QVector<double>& in,
                          QVector<double>& outRe,
                          QVector<double>& outIm) const;

    /** @brief In-place radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, int sign) const;

    /** @brief Peak-hold decay envelope */
    QVector<double> peakHoldDecay(const QVector<double>& envelope) const;

    /** @brief Unwrap phase to remove discontinuities */
    static QVector<double> unwrapPhase(const QVector<double>& phase);
};
