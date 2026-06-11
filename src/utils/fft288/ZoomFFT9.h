/**
 * @file ZoomFFT9.h
 * @brief 缩放FFT(多相抽取滤波与复数带通窄带高分辨率频谱缩放) — Zoom FFT with Polyphase Decimation Filter and Complex Band-pass for High-resolution Spectral Zooming in Narrow Bands
 *
 * 功能: 实现缩放FFT(Zoom FFT)，采用多相抽取滤波(polyphase decimation filter)
 *       与复数带通(complex band-pass)实现窄带高分辨率频谱缩放(high-resolution spectral zooming in narrow bands)。
 *
 * 协作: SlidingDFT11(滑动DFT) / FFT10(FFT) / NumberTheoreticTransform8(数论变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(多相抽取滤波与复数带通窄带高分辨率频谱缩放)
 */
class ZoomFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Zoom configuration */
    struct ZoomConfig {
        double centerFreq = 0.0;        // Center frequency in normalized units [0, 0.5]
        double bandwidth = 0.1;         // Zoom bandwidth in normalized units
        int decimationFactor = 8;       // D (must be power of 2)
        int fftSize = 1024;             // FFT size after decimation
        int filterTaps = 64;            // Polyphase filter taps per branch
    };

    /** @brief Zoom spectrum result */
    struct ZoomResult {
        QVector<double> magnitude;      // Magnitude spectrum
        QVector<double> phase;          // Phase spectrum
        QVector<double> frequencies;    // Frequency bins (Hz, if sampleRate set)
        double effectiveBandwidth = 0.0;
        double frequencyResolution = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastFFTSize = 0;
        int lastDecimation = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT9(QObject *parent = nullptr);
    ~ZoomFFT9() override;

    void setConfig(const ZoomConfig& cfg);
    void setSampleRate(double rate);

    /** @brief Process input and return zoomed spectrum */
    ZoomResult process(const QVector<double>& input);

    /** @brief Complex mixer: frequency shift input by -fc */
    QVector<double> complexMix(const QVector<double>& input) const;

    /** @brief Polyphase decimation filter */
    QVector<double> polyphaseDecimate(const QVector<double>& input) const;

    /** @brief Compute FFT on decimated data */
    void computeFFT(QVector<double>& real, QVector<double>& imag, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void zoomDone(int fftSize, int decimation, double timeMs);

private:
    ZoomConfig m_config;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_filterCoeffs;     // Prototype low-pass filter coefficients

    /** @brief Design prototype low-pass filter for decimation */
    void designFilter();

    /** @brief Bit-reversal permutation */
    static void bitReverse(QVector<double>& re, QVector<double>& im, int n);

    /** @brief Next power of 2 */
    static int nextPow2(int n);

    /** @brief Sinc function */
    static double sinc(double x);

    /** @brief Blackman window */
    static double blackman(int i, int N);
};
