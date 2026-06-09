/**
 * @file BruunFFT9.h
 * @brief Bruun FFT(递归多项式分解+N=2^k实输出优化) — Bruun FFT with Recursive Polynomial Factorization and Real-Output Optimization for N=2^k Transforms
 *
 * 功能: 实现Bruun FFT算法(Bruun's FFT)，利用递归多项式分解(recursive
 *       polynomial factorization)将z^N-1分解为二次因子，实输出优化
 *       (real-output optimization)针对N=2^k变换减少计算量。
 *
 * 协作: PrimeFactorFFT9(素因子FFT) / SplitRadixFFT8(分裂基FFT) / BruunFFT9(Bruun FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun FFT(递归多项式分解+实输出优化)
 */
class BruunFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numFactorizations = 0;
        int polynomialOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT9(QObject *parent = nullptr);
    ~BruunFFT9() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n);

    /** @brief Forward real-to-complex FFT (returns interleaved [re,im,...]) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse complex-to-real FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool forward, double timeMs);

private:
    int m_size = 0;

    /** @brief Quadratic factor coefficients for decomposition */
    struct QuadFactor {
        double a = 0.0;   // z^2 coefficient
        double b = 0.0;   // z^1 coefficient
        double c = 1.0;   // z^0 coefficient
    };

    QVector<QuadFactor> m_factors;  // Precomputed factorization
    QVector<double> m_twiddles;     // Twiddle factors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factor z^N - 1 into quadratic factors */
    void factorizePolynomial(int n);

    /** @brief Recursive Bruun butterfly */
    void butterfly(QVector<double>& re, QVector<double>& im,
                    int start, int length, int stride) const;

    /** @brief Real-output optimized butterfly */
    void realButterfly(QVector<double>& re, QVector<double>& im,
                        int n) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& data) const;

    /** @brief Check if n is power of 2 */
    static bool isPowerOf2(int n);
};
