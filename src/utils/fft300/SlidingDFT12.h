/**
 * @file SlidingDFT12.h
 * @brief 滑动离散傅里叶变换(Goertzel递归更新与频谱泄漏窗补偿实现连续实时频率监测) — Sliding DFT with Goertzel-based Recursive Update and Spectral Leakage Window Compensation for Continuous Real-time Frequency Monitoring
 *
 * 功能: 实现滑动离散傅里叶变换(Sliding DFT)，采用Goertzel递归更新(Goertzel-based recursive update)
 *       与频谱泄漏窗补偿(spectral leakage window compensation)实现连续实时频率监测(continuous real-time frequency monitoring)。
 *
 * 协作: ZoomFFT10(缩放FFT) / GoertzelFilter(戈泽尔滤波) / SplitRadixFFT11(分裂基FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class SlidingDFT12 : public QObject {
    Q_OBJECT

public:
    /** @brief Bin state for Goertzel recursive computation */
    struct BinState {
        double coeff1 = 0.0;      // 2*cos(2*pi*k/N)
        double s0 = 0.0;          // current state
        double s1 = 0.0;          // previous state
        double s2 = 0.0;          // two-steps-back state
        double magnitude = 0.0;
        double phase = 0.0;
    };

    /** @brief Spectral output for all monitored bins */
    struct SpectrumSnapshot {
        QVector<double> magnitudes;
        QVector<double> phases;
        QVector<double> frequencies;
        double timestamp = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalUpdates = 0;
        int numBins = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT12(QObject *parent = nullptr);
    ~SlidingDFT12() override;

    void setSampleRate(double rate);
    void setTransformSize(int N);

    /** @brief Configure which frequency bins to monitor */
    void setMonitoredBins(const QVector<double>& frequenciesHz);

    /** @brief Initialize with a block of samples (primes the sliding window) */
    void initialize(const QVector<double>& samples);

    /** @brief Push a single new sample, sliding the window by one */
    void pushSample(double sample);

    /** @brief Get current spectrum snapshot */
    SpectrumSnapshot snapshot() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int bins, double timestamp, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_N = 1024;
    QVector<BinState> m_bins;
    QVector<double> m_window;       // Hann window compensation factors
    QVector<double> m_circularBuf;  // circular sample buffer
    int m_writePos = 0;
    bool m_initialized = false;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Goertzel coefficient for bin k */
    double goertzelCoeff(int k) const;

    /** @brief Apply Hann window compensation factor at index */
    double windowCompensation(int idx) const;

    /** @brief Precompute window compensation table */
    void precomputeWindow();

    /** @brief Initialize bin states for monitored frequencies */
    void initBins();
};
