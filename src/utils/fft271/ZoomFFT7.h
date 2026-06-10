/**
 * @file ZoomFFT7.h
 * @brief 缩放FFT(复数频移抽取与窄带谱分析高分辨率频率缩放) — Zoom FFT with Complex Band-Shift Decimation and Narrowband Spectral Analysis for High-Resolution Frequency Zooming
 *
 * 功能: 实现缩放FFT(Zoom FFT)，采用复数频移抽取(complex band-shift decimation)
 *       与窄带谱分析(narrowband spectral analysis)实现高分辨率频率缩放(high-resolution frequency zooming)。
 *
 * 协作: DistributedArithmetic10(分布式算术) / FFT10(快速傅里叶变换) / FFTShift9(频移)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(复数频移抽取与窄带谱分析高分辨率频率缩放)
 */
class ZoomFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputSize = 0;
        int fftSize = 0;
        int decimationFactor = 1;
        double centerFreq = 0.0;
        double bandwidth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Complex sample pair (real, imag) */
    struct Complex {
        double re = 0.0;
        double im = 0.0;
    };

    explicit ZoomFFT7(QObject *parent = nullptr);
    ~ZoomFFT7() override;

    /** @brief Set center frequency for zoom band (normalized 0..1) */
    void setCenterFrequency(double fc);

    /** @brief Set decimation factor D (bandwidth = fs/D) */
    void setDecimationFactor(int d);

    /** @brief Set FFT size for output spectrum */
    void setFFTSize(int n);

    /** @brief Process real-valued input, returns complex spectrum */
    QVector<Complex> process(const QVector<double>& input);

    /** @brief Get magnitude spectrum in dB */
    QVector<double> magnitudeSpectrum() const;

    /** @brief Get frequency axis for zoomed band */
    QVector<double> frequencyAxis(double sampleRate) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumReady(int fftSize, double centerFreq, double timeMs);

private:
    double m_centerFreq = 0.25;     // normalized center
    int m_decimation = 4;
    int m_fftSize = 1024;

    QVector<Complex> m_spectrum;
    QVector<double> m_magnitude;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Frequency shift (mix down to baseband) */
    QVector<Complex> frequencyShift(const QVector<double>& input) const;

    /** @brief Low-pass filter and decimate */
    QVector<Complex> decimate(const QVector<Complex>& shifted) const;

    /** @brief Apply FFT on complex data (Cooley-Tukey radix-2) */
    QVector<Complex> fft(const QVector<Complex>& data) const;

    /** @brief Compute twiddle factor */
    Complex twiddle(int k, int n, bool inverse) const;
};
