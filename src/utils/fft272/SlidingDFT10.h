/**
 * @file SlidingDFT10.h
 * @brief 滑动DFT(Goertzel递归更新与循环缓冲区O(1)逐样本频谱bin计算) — Sliding DFT with Goertzel-Based Recursive Update and Circular Buffer for O(1) Per-Sample Spectral Bin Computation
 *
 * 功能: 实现滑动DFT(Sliding DFT)，采用Goertzel递归更新(Goertzel-based recursive update)
 *       与循环缓冲区(circular buffer)实现O(1)逐样本频谱bin计算(O(1) per-sample spectral bin computation)。
 *
 * 协作: ZoomFFT7(缩放FFT) / Goertzel5(Goertzel算法) / FFTShift9(频移)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(Goertzel递归更新与循环缓冲区O(1)逐样本频谱bin计算)
 */
class SlidingDFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Complex sample */
    struct Complex {
        double re = 0.0;
        double im = 0.0;
    };

    explicit SlidingDFT10(QObject *parent = nullptr);
    ~SlidingDFT10() override;

    /** @brief Set transform block size N */
    void setBlockSize(int n);

    /** @brief Set target frequency bins to monitor */
    void setBins(const QVector<int>& bins);

    /** @brief Push a single sample, returns true when block is complete */
    bool pushSample(double sample);

    /** @brief Process entire block at once */
    QVector<Complex> processBlock(const QVector<double>& input);

    /** @brief Get current magnitude for all monitored bins */
    QVector<double> magnitudes() const;

    /** @brief Get current phase for all monitored bins (radians) */
    QVector<double> phases() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockReady(int blockSize, int numBins, double timeMs);

private:
    int m_blockSize = 1024;
    int m_writePos = 0;

    /** @brief Per-bin Goertzel state */
    struct BinState {
        int k = 0;              // target bin index
        double coeff1 = 0.0;    // 2*cos(2*pi*k/N)
        double coeff2 = 0.0;    // -1 (exp(-j*2*pi*k/N) imaginary)
        double s0 = 0.0;        // current accumulator
        double s1 = 0.0;        // previous accumulator
        double s2 = 0.0;        // two-steps-back accumulator
        Complex output;         // latest output
    };

    QVector<BinState> m_bins;
    QVector<double> m_circularBuf; // circular buffer of size N

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize Goertzel coefficients for a bin */
    void initBin(BinState& state, int k, int n);

    /** @brief Update all bins with one sample addition and one removal */
    void updateBins(double newSample, double oldSample);
};
