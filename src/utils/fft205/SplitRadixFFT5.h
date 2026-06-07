/**
 * @file SplitRadixFFT5.h
 * @brief 分裂基FFT(共轭对旋转因子优化+缓存友好迭代实现) — Split-Radix FFT with Conjugate-Pair Twiddle Factor Optimization and Cache-Friendly Iterative Implementation
 *
 * 功能: 实现分裂基FFT算法，支持共轭对旋转因子优化、
 *       缓存友好的迭代蝶形网络和自动2^n对齐。
 *
 * 协作: BruunFFT5(Bruun FFT) / WinogradFFT6(Winograd FFT) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT(共轭对旋转因子优化+缓存友好迭代实现)
 */
class SplitRadixFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT5(QObject *parent = nullptr);
    ~SplitRadixFFT5() override;

    /** @brief Set transform size (must be power of 2, minimum 4) */
    void setTransformSize(int n);

    /** @brief Forward complex FFT: interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse complex FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Generate conjugate-pair optimized twiddle factors */
    void computeTwiddles(int n);

    /** @brief Execute split-radix butterfly network iteratively */
    void butterflyIterative(QVector<double>& data, bool inverse) const;

    /** @brief Bit-reversal permutation in-place */
    void bitReversePermute(QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_size = 64;

    // Conjugate-pair twiddle factors: only N/4 unique pairs stored
    QVector<double> m_cosTwiddle;  // cos(2*pi*k/N) for k=0..N/4-1
    QVector<double> m_sinTwiddle;  // sin(2*pi*k/N) for k=0..N/4-1
    QVector<int> m_bitRevTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute bit-reversal lookup table */
    void buildBitRevTable(int n);

    /** @brief Compute bit-reversal of an integer */
    static int bitReverse(int x, int log2n);
};
