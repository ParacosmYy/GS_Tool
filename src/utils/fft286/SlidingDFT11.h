/**
 * @file SlidingDFT11.h
 * @brief 滑动DFT(加窗递归更新与频谱泄漏补偿) — Sliding DFT with Windowed Recursive Update and Spectral Leakage Compensation for Continuous Frequency Monitoring
 *
 * 功能: 实现滑动DFT(Sliding DFT)，采用加窗递归更新(windowed recursive update)
 *       与频谱泄漏补偿(spectral leakage compensation)实现连续频率监测(continuous frequency monitoring)。
 *
 * 协作: ZoomFFT8(缩放FFT) / Goertzel7(Goertzel算法) / FFT10(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(加窗递归更新与频谱泄漏补偿)
 */
class SlidingDFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief DFT configuration */
    struct DFTConfig {
        int fftSize = 256;           // Transform size N
        int hopSize = 1;             // Samples between updates
        double windowCorrection = 1.0; // Amplitude correction factor
        bool applyWindow = true;
    };

    /** @brief Spectrum frame result */
    struct SpectrumFrame {
        QVector<double> magnitude;
        QVector<double> phase;
        double timestamp = 0.0;       // Sample index
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT11(QObject *parent = nullptr);
    ~SlidingDFT11() override;

    void setConfig(const DFTConfig& cfg);

    /** @brief Initialize DFT for specific bin indices */
    void initBins(const QVector<int>& bins);

    /** @brief Push single sample and update DFT recursively */
    void pushSample(double sample);

    /** @brief Push block of samples, get per-frame spectra */
    QVector<SpectrumFrame> processBlock(const QVector<double>& block);

    /** @brief Get current magnitude spectrum (all bins or initialized) */
    SpectrumFrame currentSpectrum() const;

    /** @brief Reset sliding window to zero state */
    void resetWindow();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameReady(int frameIdx, double timeMs);

private:
    DFTConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    int m_writeIdx = 0;
    quint64 m_sampleCount = 0;

    QVector<double> m_circularBuf;      // Circular input buffer
    QVector<double> m_window;           // Window coefficients
    QVector<double> m_re;               // Real part of DFT bins
    QVector<double> m_im;               // Imaginary part of DFT bins

    QVector<double> m_prevRe;           // Previous frame real (for leakage comp)
    QVector<double> m_prevIm;           // Previous frame imaginary

    QVector<int> m_activeBins;          // Subset of bins to compute (empty = all)

    /** @brief Design Hann window */
    void designWindow();

    /** @brief Compute twiddle factor for bin k */
    void twiddle(int k, double& wr, double& wi) const;

    /** @brief Single recursive DFT update on new sample */
    void recursiveUpdate(double xNew, double xOld);

    /** @brief Apply spectral leakage compensation */
    void applyLeakageCompensation();
};
