/**
 * @file SlidingDFT8.h
 * @brief 滑动DFT(递归Goertzel更新+梳状滤波器极点校正保证稳定性) — Sliding DFT with Recursive Goertzel Update and Guaranteed Stability via Comb Filter Pole Correction
 *
 * 功能: 实现滑动DFT(Sliding DFT)，通过递归Goertzel更新(recursive Goertzel
 *       update)实现每样本O(1)频谱计算，使用梳状滤波器极点校正(comb filter
 *       pole correction)确保长期数值稳定性。
 *
 * 协作: ZoomFFT5(缩放FFT) / BruunFFT8(Bruun FFT) / GoertzelBank6(Goertzel组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(递归Goertzel更新+梳状滤波器极点校正保证稳定性)
 */
class SlidingDFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numBins = 0;
        double correctionFactor = 0.0;
        int numCorrections = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT8(QObject *parent = nullptr);
    ~SlidingDFT8() override;

    /** @brief Set transform block size N */
    void setBlockSize(int n);

    /** @brief Set which frequency bins to compute (1-indexed, 1..N/2) */
    void setBins(const QVector<int>& bins);

    /** @brief Set stability correction interval (samples between corrections) */
    void setCorrectionInterval(int samples);

    /** @brief Process a single sample, update all bins */
    void pushSample(double sample);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& samples);

    /** @brief Get current magnitude for a bin */
    double magnitude(int bin) const;

    /** @brief Get current phase for a bin */
    double phase(int bin) const;

    /** @brief Get all bin magnitudes */
    QVector<double> magnitudes() const;

    /** @brief Reset sliding state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int numSamples, int numBins, double timeMs);

private:
    int m_blockSize = 64;
    int m_correctionInterval = 1024;
    int m_sampleCount = 0;

    /** @brief Per-bin Goertzel state */
    struct BinState {
        int k = 0;           // bin index
        double coeff = 0.0;  // 2*cos(2*pi*k/N)
        double s0 = 0.0;     // current Goertzel state
        double s1 = 0.0;     // previous state
        double s2 = 0.0;     // state before s1
        double xOld = 0.0;   // oldest sample in the window
    };

    QVector<BinState> m_bins;
    QVector<double> m_circularBuf;
    int m_bufPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize bin coefficients */
    void initBins();

    /** @brief Apply comb filter pole correction for stability */
    void applyCorrection();
};
