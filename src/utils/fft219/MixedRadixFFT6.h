/**
 * @file MixedRadixFFT6.h
 * @brief 混合基数FFT(自动因子分解调度+旋转因子预计算) — Mixed-Radix FFT with Automatic Factorization Scheduling and Twiddle Precomputation for Arbitrary Composite N
 *
 * 功能: 实现混合基数FFT算法，自动分解N为素因子组合，
 *       预计算旋转因子，支持任意复合长度的高效变换。
 *
 * 协作: HexFFT6(六角FFT) / WinogradFFT4(Winograd FFT) / SlidingDFT6(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基数FFT(自动因子分解+旋转因子预计算)
 */
class MixedRadixFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT6(QObject *parent = nullptr);
    ~MixedRadixFFT6() override;

    /** @brief Set transform size (any composite number) */
    void setParameters(int size);

    /** @brief Forward transform, returns complex interleaved [re,im,...] */
    QVector<double> forward(const QVector<double>& realInput);

    /** @brief Inverse transform from complex interleaved */
    QVector<double> inverse(const QVector<double>& complexInterleaved);

    /** @brief Get factorization of current size */
    QVector<int> factors() const;

    /** @brief Compute magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& realInput);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int numFactors, double timeMs);

private:
    int m_size = 0;
    QVector<int> m_factors;   // prime factor decomposition
    QVector<int> m_perm;      // index permutation

    // Precomputed twiddle factors [re,im] pairs
    QVector<double> m_twiddles;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into prime factors */
    QVector<int> factorize(int n) const;

    /** @brief Compute index permutation from factorization */
    void computePermutation();

    /** @brief Precompute all twiddle factors */
    void computeTwiddles();

    /** @brief Apply a single radix-p DFT butterfly */
    void radixButterfly(QVector<double>& re, QVector<double>& im,
                        int radix, int base, int stride, int groupSize,
                        bool inverse) const;

    /** @brief Main mixed-radix transform */
    void transform(QVector<double>& re, QVector<double>& im, bool inverse);
};
