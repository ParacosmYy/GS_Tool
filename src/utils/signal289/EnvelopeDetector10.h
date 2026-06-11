/**
 * @file EnvelopeDetector10.h
 * @brief 包络检测器(解析信号分解与频率加权Hilbert变换瞬时幅度估计) — Envelope Detector with Analytic Signal Decomposition and Instantaneous Amplitude Estimation via Frequency-weighted Hilbert Transform
 *
 * 功能: 实现包络检测器(envelope detector)，采用解析信号分解(analytic signal decomposition)
 *       与频率加权Hilbert变换(frequency-weighted Hilbert transform)实现瞬时幅度估计(instantaneous amplitude estimation)。
 *
 * 协作: Limiter13(限制器) / Compressor12(压缩器) / Demodulator9(解调器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(解析信号分解与频率加权Hilbert变换瞬时幅度估计)
 */
class EnvelopeDetector10 : public QObject {
    Q_OBJECT

public:
    /** @brief Detector configuration */
    struct DetectorConfig {
        double sampleRate = 44100.0;
        int fftSize = 2048;
        double lowFreqWeight = 1.0;     // Weight for low frequencies
        double highFreqWeight = 0.5;    // Weight for high frequencies
        double freqCutoff = 0.0;        // Hz (0 = auto)
    };

    /** @brief Envelope result */
    struct EnvelopeResult {
        QVector<double> envelope;       // Instantaneous amplitude
        QVector<double> instPhase;      // Instantaneous phase
        QVector<double> instFreq;       // Instantaneous frequency
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector10(QObject *parent = nullptr);
    ~EnvelopeDetector10() override;

    void setConfig(const DetectorConfig& cfg);

    /** @brief Compute envelope via frequency-weighted Hilbert transform */
    EnvelopeResult detect(const QVector<double>& input);

    /** @brief Quick envelope using simple analytic signal (no freq weighting) */
    QVector<double> detectSimple(const QVector<double>& input) const;

    /** @brief Compute analytic signal (real + imaginary) */
    void analyticSignal(const QVector<double>& input,
                        QVector<double>& realOut,
                        QVector<double>& imagOut);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionDone(int samples, double peakEnv, double timeMs);

private:
    DetectorConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_hilbertCoeffs;    // FIR Hilbert filter coefficients
    QVector<double> m_freqWeights;      // Frequency-dependent weights

    /** @brief Design FIR Hilbert transformer via frequency sampling */
    void designHilbertFilter();

    /** @brief Compute frequency-dependent weights */
    void computeFreqWeights();

    /** @brief In-place FFT (Cooley-Tukey radix-2) */
    void fft(QVector<double>& re, QVector<double>& im, int n, bool inverse);

    /** @brief Next power of 2 */
    static int nextPow2(int n);

    /** @brief Apply frequency-weighted Hilbert in spectral domain */
    void applyWeightedHilbert(QVector<double>& re, QVector<double>& im, int n);
};
