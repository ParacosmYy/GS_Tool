/**
 * @file SlidingDFT7.h
 * @brief 滑动DFT(保证稳定性窗递归+逐频点噪声底跟踪实时频谱) — Sliding DFT with Guaranteed-stability Windowed Recursion and Bin-wise Noise Floor Tracking for Real-time Spectrum
 *
 * 功能: 实现滑动DFT(Sliding DFT)，采用保证稳定性的窗递归(windowed recursion)算法，
 *       并逐频点(bin-wise)跟踪噪声底(noise floor)，适用于实时频谱分析。
 *
 * 协作: FFTCore5(FFT核心) / ZoomFFT4(缩放FFT) / Goertzel6(Goertzel算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(稳定窗递归+逐频点噪声底)
 */
class SlidingDFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-bin tracking data */
    struct BinInfo {
        double real = 0.0;
        double imag = 0.0;
        double magnitude = 0.0;
        double noiseFloor = 0.0;
        double peakHold = 0.0;
        int peakHoldCount = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int numBins = 0;
        int samplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT7(QObject *parent = nullptr);
    ~SlidingDFT7() override;

    /** @brief Initialize with transform size */
    bool configure(int fftSize);

    /** @brief Push single sample through sliding DFT */
    void pushSample(double sample);

    /** @brief Push a block of samples */
    void pushBlock(const QVector<double>& samples);

    /** @brief Get current magnitude spectrum (N/2 bins) */
    QVector<double> magnitudes() const;

    /** @brief Get noise floor estimate per bin */
    QVector<double> noiseFloor() const;

    /** @brief Reset all bins and circular buffer */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int bins, double timeMs);

private:
    int m_fftSize = 64;
    int m_pos = 0;              // circular buffer position

    QVector<double> m_buffer;   // circular sample buffer
    QVector<BinInfo> m_bins;    // per-bin state (N/2)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute twiddle factor for bin k */
    void twiddle(int k, double& cosW, double& sinW) const;

    /** @brief Update noise floor estimate using exponential decay */
    void updateNoiseFloor(int bin, double magnitude);

    /** @brief Update peak hold with decay */
    void updatePeakHold(int bin, double magnitude);
};

