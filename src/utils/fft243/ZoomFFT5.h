/**
 * @file ZoomFFT5.h
 * @brief 缩放FFT(带通抽取+复数解调窄带高分辨率频谱分析) — Zoom FFT with Bandpass Decimation and Complex Demodulation for High-Resolution Spectral Analysis in Narrow Bands
 *
 * 功能: 实现缩放FFT(Zoom FFT)，通过带通抽取(bandpass decimation)对目标频段
 *       进行下采样，结合复数解调(complex demodulation)将感兴趣频段搬移至基带，
 *       实现窄带内高分辨率频谱分析(high-resolution spectral analysis)。
 *
 * 协作: DistributedArithmetic8(分布式算术) / BruunFFT8(Bruun FFT) / WHT7(沃尔什变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(带通抽取+复数解调窄带高分辨率频谱分析)
 */
class ZoomFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputSize = 0;
        int fftSize = 0;
        int decimationFactor = 0;
        double centerFreqHz = 0.0;
        double bandwidthHz = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT5(QObject *parent = nullptr);
    ~ZoomFFT5() override;

    /** @brief Set center frequency of zoom band in Hz */
    void setCenterFrequency(double freq);

    /** @brief Set decimation factor */
    void setDecimationFactor(int d);

    /** @brief Set FFT size for the zoomed analysis */
    void setFFTSize(int n);

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Process input and return zoomed spectrum magnitude */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get frequency axis for the zoomed spectrum */
    QVector<double> frequencyAxis() const;

    /** @brief Set window type: 0=rect, 1=hann, 2=hamming */
    void setWindowType(int type);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void zoomCompleted(int inputSize, int zoomBins, double timeMs);

private:
    double m_centerFreq = 1000.0;
    int m_decimation = 4;
    int m_fftSize = 1024;
    double m_sampleRate = 44100.0;
    int m_windowType = 1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Complex demodulation: shift center freq to baseband */
    void demodulate(const QVector<double>& in,
                    QVector<double>& outRe, QVector<double>& outIm) const;

    /** @brief Low-pass filter + decimate */
    void decimate(const QVector<double>& re, const QVector<double>& im,
                  QVector<double>& outRe, QVector<double>& outIm) const;

    /** @brief Apply window function */
    void applyWindow(QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place radix-2 complex FFT */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Design simple FIR low-pass for decimation */
    QVector<double> designLowpass(int len, double cutoff) const;
};
