/**
 * @file SplitRadixFFT8.h
 * @brief 分裂基FFT(共轭对旋转因子优化+N=4^k长度分裂基蝶形核) — Split-Radix FFT with Conjugate-Pair Twiddle Factor Optimization and Split-Radix Butterfly Kernel for N=4^k Lengths
 *
 * 功能: 实现分裂基FFT(Split-Radix FFT)，利用共轭对旋转因子优化(conjugate-
 *       pair twiddle factor optimization)减少三角函数计算，使用分裂基蝶形核
 *       (split-radix butterfly kernel)处理N=4^k长度的高效变换。
 *
 * 协作: MixedRadixFFT8(混合基FFT) / HexFFT8(六边形FFT) / SlidingDFT8(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT(共轭对旋转因子优化+分裂基蝶形核)
 */
class SplitRadixFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex sample pair [real, imag] */
    using Complex = QPair<double, double>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int radixSplits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT8(QObject *parent = nullptr);
    ~SplitRadixFFT8() override;

    /** @brief Perform forward split-radix FFT */
    QVector<Complex> forward(const QVector<double>& realInput);

    /** @brief Perform forward FFT on complex input */
    QVector<Complex> forwardComplex(const QVector<Complex>& input);

    /** @brief Perform inverse FFT */
    QVector<Complex> inverse(const QVector<Complex>& spectrum);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors with conjugate symmetry */
    QVector<Complex> computeTwiddles(int N) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<Complex>& data) const;

    /** @brief Core split-radix FFT butterfly */
    void splitRadixCore(QVector<Complex>& data, int N,
                         const QVector<Complex>& twiddles) const;

    /** @brief Split-radix butterfly for a sub-problem of size n at offset */
    void butterfly(QVector<Complex>& data, int offset, int n, int stride,
                    const QVector<Complex>& twiddles) const;

    /** @brief Length-4 DFT kernel */
    void dft4(Complex& x0, Complex& x1, Complex& x2, Complex& x3) const;

    /** @brief Length-2 DFT kernel */
    void dft2(Complex& x0, Complex& x1) const;
};
