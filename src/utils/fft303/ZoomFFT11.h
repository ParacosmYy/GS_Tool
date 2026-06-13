/**
 * @file ZoomFFT11.h
 * @brief 缩放FFT(滑动窗口累加与多相带通实现实时连续窄带频谱监测) — Zoom FFT with Sliding Window Accumulation and Polyphase Bandpass for Continuous Narrowband Spectral Monitoring in Real-Time
 *
 * 功能: 实现缩放FFT(zoom FFT)，采用滑动窗口累加(sliding window accumulation)
 *       与多相带通(polyphase bandpass)实现实时连续窄带频谱监测(continuous narrowband spectral monitoring in real-time)。
 *
 * 协作: MixedRadixFFT12(混合基FFT) / SlidingDFT12(滑动DFT) / GoertzelFilter(Goertzel滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

class ZoomFFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex number */
    struct Complex {
        double real = 0.0;
        double imag = 0.0;
        Complex operator+(const Complex& o) const { return {real + o.real, imag + o.imag}; }
        Complex operator-(const Complex& o) const { return {real - o.real, imag - o.imag}; }
        Complex operator*(const Complex& o) const { return {real*o.real - imag*o.imag, real*o.imag + imag*o.real}; }
    };

    /** @brief Zoom configuration */
    struct ZoomConfig {
        double centerFreq = 0.0;     // Hz
        double bandwidth = 1000.0;   // Hz
        int zoomFactor = 8;          // Decimation ratio
        int fftSize = 512;
    };

    /** @brief Spectral result */
    struct SpectrumResult {
        QVector<double> magnitudes;
        QVector<double> frequencies;
        double peakFreq = 0.0;
        double peakMag = 0.0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        double avgProcessingTimeMs = 0.0;
        double lastPeakFreq = 0.0;
    };

    explicit ZoomFFT11(QObject *parent = nullptr);
    ~ZoomFFT11() override;

    void setSampleRate(double rate);
    void setZoomConfig(const ZoomConfig& config);

    /** @brief Process a block with polyphase bandpass + decimation + FFT */
    SpectrumResult process(const QVector<double>& input);

    /** @brief Process with sliding window accumulation */
    SpectrumResult processSliding(const QVector<double>& input);

    /** @brief Get zoomed frequency axis */
    QVector<double> frequencyAxis() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumReady(double peakFreq, double peakMag, double timeMs);

private:
    double m_sampleRate = 44100.0;
    ZoomConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Polyphase filter state
    QVector<QVector<double>> m_polyCoeffs;
    QVector<QVector<double>> m_polyState;
    QVector<double> m_window;

    /** @brief Design polyphase bandpass filter coefficients */
    void designPolyphaseFilter();

    /** @brief Apply polyphase filter and decimate */
    QVector<double> polyphaseDecimate(const QVector<double>& input);

    /** @brief Frequency-shift signal to baseband */
    QVector<Complex> frequencyShift(const QVector<double>& input) const;

    /** @brief Compute FFT on complex baseband signal */
    QVector<Complex> fft(const QVector<Complex>& input) const;

    /** @brief Bit-reverse permutation for FFT */
    void bitReverse(QVector<Complex>& data) const;

    /** @brief Apply window function (Hann) */
    QVector<double> applyWindow(const QVector<double>& data) const;
};
