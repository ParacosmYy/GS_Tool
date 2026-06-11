/**
 * @file MixedRadixFFT11.h
 * @brief 混合基FFT(自动素因子分解与自排序原地蝶形运算支持任意复合长度) — Mixed-radix FFT with Automatic Prime Factor Decomposition and Self-sorting In-place Butterfly for Arbitrary Composite Lengths
 *
 * 功能: 实现混合基FFT(Mixed-radix FFT)，采用自动素因子分解(automatic prime factor decomposition)
 *       与自排序原地蝶形(self-sorting in-place butterfly)实现任意复合长度(arbitrary composite lengths)。
 *
 * 协作: ZoomFFT9(缩放FFT) / SlidingDFT11(滑动DFT) / FFT10(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基FFT(自动素因子分解与自排序原地蝶形运算支持任意复合长度)
 */
class MixedRadixFFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        QVector<double> magnitude;
        QVector<double> phase;
        int n = 0;
        QVector<int> factors;        // Prime factors of n
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastN = 0;
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT11(QObject *parent = nullptr);
    ~MixedRadixFFT11() override;

    /** @brief Perform forward FFT on real input, zero-imag part */
    FFTResult forward(const QVector<double>& input);

    /** @brief Perform forward FFT on complex input */
    FFTResult forwardComplex(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Perform inverse FFT */
    FFTResult inverse(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Decompose n into prime factors */
    QVector<int> factorize(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fftDone(int n, int numFactors, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Core in-place mixed-radix FFT (Cooley-Tukey generalized) */
    void mixedRadixCore(QVector<double>& re, QVector<double>& im,
                        int n, const QVector<int>& factors, bool inverse);

    /** @brief Compute digit-reversed index for given factors */
    int digitReverse(int idx, int n, const QVector<int>& factors) const;

    /** @brief Compute DFT of small prime size (2, 3, 5, 7) directly */
    void smallPrimeDFT(QVector<double>& re, QVector<double>& im,
                       int start, int stride, int p, double sign) const;

    /** @brief Twiddle factor: exp(j*2*pi*k/n) */
    void twiddle(int k, int n, double sign, double& wRe, double& wIm) const;

    /** @brief Next composite number (product of small primes) >= n */
    int nextComposite(int n) const;

    /** @brief Check if n is a product of small primes {2,3,5,7} */
    bool isSmallComposite(int n) const;
};
