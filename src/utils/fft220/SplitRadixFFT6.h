/**
 * @file SplitRadixFFT6.h
 * @brief 分裂基FFT(共轭对旋转因子优化+迭代深度优先蝶形调度) — Split-Radix FFT with Conjugate-Pair Twiddle Optimization and Iterative Depth-First Butterfly Scheduling
 *
 * 功能: 实现分裂基FFT算法，利用共轭对旋转因子减少乘法次数，
 *       迭代深度优先蝶形调度提高缓存局部性。
 *
 * 协作: MixedRadixFFT6(混合基数FFT) / WinogradFFT4(Winograd FFT) / SlidingDFT6(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT(共轭对优化+深度优先蝶形)
 */
class SplitRadixFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int butterflyCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT6(QObject *parent = nullptr);
    ~SplitRadixFFT6() override;

    /** @brief Set transform size (must be power of 2) */
    void setParameters(int size);

    /** @brief Forward transform, returns complex interleaved [re,im,...] */
    QVector<double> forward(const QVector<double>& realInput);

    /** @brief Inverse transform from complex interleaved */
    QVector<double> inverse(const QVector<double>& complexInterleaved);

    /** @brief Compute magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& realInput);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int butterflies, double timeMs);

private:
    int m_size = 0;
    int m_log2n = 0;

    // Conjugate-pair twiddle factors: cos and sin tables
    QVector<double> m_cosTable;
    QVector<double> m_sinTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute bit-reversal permutation index */
    int bitReverse(int x, int log2n) const;

    /** @brief Precompute conjugate-pair twiddle factors */
    void computeTwiddles();

    /** @brief Iterative split-radix butterfly (depth-first) */
    void splitRadixButterfly(QVector<double>& re, QVector<double>& im,
                              bool inverse) const;
};
