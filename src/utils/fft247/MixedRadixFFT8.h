/**
 * @file MixedRadixFFT8.h
 * @brief 混合基FFT(自动分解因子+Good排列的任意复合长度变换) — Mixed-Radix FFT with Automatic Factorization and Good's Permutation for Arbitrary Composite Lengths
 *
 * 功能: 实现混合基FFT(Mixed-Radix FFT)，支持自动分解因子(automatic
 *       factorization)将任意复合长度分解为质因子，使用Good排列(Good's
 *       permutation)进行索引重排，实现任意复合长度的高效变换。
 *
 * 协作: HexFFT8(六边形FFT) / NumberTheoreticTransform5(数论变换) / SlidingDFT8(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基FFT(自动分解因子+Good排列)
 */
class MixedRadixFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex sample pair [real, imag] */
    using Complex = QPair<double, double>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT8(QObject *parent = nullptr);
    ~MixedRadixFFT8() override;

    /** @brief Perform forward FFT (returns complex spectrum) */
    QVector<Complex> forward(const QVector<double>& realInput);

    /** @brief Perform forward FFT on complex input */
    QVector<Complex> forwardComplex(const QVector<Complex>& input);

    /** @brief Perform inverse FFT */
    QVector<Complex> inverse(const QVector<Complex>& spectrum);

    /** @brief Get factorization of the last transform size */
    QVector<int> lastFactorization() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    QVector<int> m_factors;  // Factorization of current N

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize n into radices (2, 3, 5 preferred) */
    QVector<int> factorize(int n) const;

    /** @brief Apply Good's permutation for index mapping */
    QVector<int> goodsPermutation(int n, const QVector<int>& factors) const;

    /** @brief Butterfly operation for radix r */
    void radixButterfly(QVector<Complex>& data, int offset, int stride,
                        int radix, bool inv) const;

    /** @brief Compute twiddle factor for given k/N */
    Complex twiddle(int k, int N, bool inv) const;

    /** @brief Core mixed-radix FFT */
    void transformInternal(QVector<Complex>& data, bool inv);
};
