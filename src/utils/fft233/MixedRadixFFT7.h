/**
 * @file MixedRadixFFT7.h
 * @brief 混合基数FFT(素因子分解树运行时选择+缓存感知旋转因子预计算) — Mixed-Radix FFT with Runtime Factor Selection via Prime Factorization Tree and Cache-Aware Twiddle Precomputation
 *
 * 功能: 实现混合基数FFT(Mixed-radix FFT)，通过素因子分解树(prime factorization tree)
 *       在运行时选择最优分解因子，并采用缓存感知(cache-aware)策略预计算旋转因子(twiddle factors)。
 *
 * 协作: HexFFT7(六角FFT) / FFTCore5(FFT核心) / NumberTheoreticTransform4(数论变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基数FFT(素因子分解树运行时选择+缓存感知旋转因子预计算)
 */
class MixedRadixFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        int twiddleCacheHits = 0;
        int twiddleCacheMisses = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT7(QObject *parent = nullptr);
    ~MixedRadixFFT7() override;

    /** @brief Configure transform size (any positive integer) */
    bool configure(int n);

    /** @brief Forward transform (complex interleaved: real0,imag0,...) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse transform (complex interleaved) */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief Get factorization tree for current N */
    QVector<int> factorTree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, double timeMs);

private:
    int m_n = 0;
    QVector<int> m_factors;       // prime factorization

    // Cache-line aware twiddle storage: real/imag in separate arrays
    QVector<double> m_twReal;
    QVector<double> m_twImag;
    QVector<int> m_twCacheTag;    // tag for cache-aware access tracking

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build prime factorization tree */
    QVector<int> primeFactorize(int n) const;

    /** @brief Cache-aware twiddle precomputation */
    void precomputeTwiddles();

    /** @brief General radix-r DIT butterfly */
    void radixButterfly(double* re, double* im, int n, int r, int stride,
                         int twBase) const;

    /** @brief Digit-reverse permutation */
    void digitReverse(double* re, double* im) const;

    /** @brief Small-N DFT for prime factors (direct computation) */
    void smallDFT(double* re, double* im, int n, int stride, int twBase) const;
};
