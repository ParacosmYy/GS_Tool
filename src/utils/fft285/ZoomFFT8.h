/**
 * @file ZoomFFT8.h
 * @brief 缩放FFT(多级抽取与复数解调的超窄带高分辨率频谱分析) — Zoom FFT with Multi-stage Decimation and Complex Demodulation for Ultra-narrowband High-resolution Spectral Analysis
 *
 * 功能: 实现缩放FFT(Zoom FFT)，采用多级抽取(multi-stage decimation)
 *       与复数解调(complex demodulation)实现超窄带高分辨率频谱分析(ultra-narrowband high-resolution spectral analysis)。
 *
 * 协作: FFT10(FFT) / Goertzel7(Goertzel算法) / DistributedArithmetic11(分布式算术)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(多级抽取与复数解调)
 */
class ZoomFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Zoom analysis parameters */
    struct ZoomConfig {
        double centerFreq = 0.0;    // Center frequency (normalized 0..0.5)
        double bandwidth = 0.1;     // Zoom bandwidth (normalized)
        int fftSize = 1024;
        int decimationStages = 3;
        int decimationFactor = 2;   // Per-stage decimation
    };

    /** @brief Zoom spectrum result */
    struct ZoomResult {
        QVector<double> magnitude;
        QVector<double> phase;
        QVector<double> freqAxis;
        double resolution = 0.0;
        double snr = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT8(QObject *parent = nullptr);
    ~ZoomFFT8() override;

    void setConfig(const ZoomConfig& cfg);

    /** @brief Analyze zoomed spectrum */
    ZoomResult analyze(const QVector<double>& input);

    /** @brief Complex demodulation: shift center freq to DC */
    QVector<double> demodulate(const QVector<double>& input);

    /** @brief Multi-stage decimation with LPF */
    QVector<double> decimate(const QVector<double>& input, int stages);

    /** @brief In-place radix-2 FFT on complex interleaved [re,im,...] */
    void fft(QVector<double>& re, QVector<double>& im) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analyzeDone(int n, double bw, double res, double timeMs);

private:
    ZoomConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_lpfCoeffs;       // Low-pass filter coefficients
    int m_phaseIndex = 0;

    /** @brief Design decimation LPF */
    void designLPF();

    /** @brief Apply FIR filter */
    QVector<double> applyFIR(const QVector<double>& input,
                              const QVector<double>& coeffs) const;

    /** @brief Downsample by factor D */
    QVector<double> downsample(const QVector<double>& input, int D) const;

    /** @brief Bit-reverse permutation index */
    int bitReverse(int x, int bits) const;
};
