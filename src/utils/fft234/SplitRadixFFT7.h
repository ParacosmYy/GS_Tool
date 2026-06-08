/**
 * @file SplitRadixFFT7.h
 * @brief 分裂基FFT(迭代就地计算+预计算旋转因子缓存最优访问) — Split-Radix FFT with Iterative In-Place Computation and Precomputed Twiddle Table for Cache-Optimal Access
 *
 * 功能: 实现分裂基FFT(split-radix FFT)，采用迭代就地(iterative in-place)计算，
 *       预计算旋转因子表(precomputed twiddle table)实现缓存最优(cache-optimal)访问。
 *
 * 协作: MixedRadixFFT7(混合基数FFT) / FFTCore5(FFT核心) / HexFFT7(六角FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT(迭代就地计算+预计算旋转因子缓存最优访问)
 */
class SplitRadixFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numButterflies = 0;
        int twiddleHits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT7(QObject *parent = nullptr);
    ~SplitRadixFFT7() override;

    /** @brief Configure transform size (must be power of 2) */
    bool configure(int n);

    /** @brief Forward transform (complex interleaved) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse transform (complex interleaved) */
    QVector<double> inverse(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, double timeMs);

private:
    int m_n = 0;
    int m_log2n = 0;

    // Precomputed twiddle table: interleaved cos/sin
    QVector<double> m_twiddle;  // [2 * n/4] for split-radix

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors for split-radix */
    void precomputeTwiddles();

    /** @brief Iterative in-place split-radix FFT core */
    void splitRadixCore(double* re, double* im, int n, bool inverse) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(double* re, double* im) const;

    /** @brief Reverse bits */
    int revBits(int x, int bits) const;
};
