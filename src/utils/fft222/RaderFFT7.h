/**
 * @file RaderFFT7.h
 * @brief Rader FFT(Winograd短卷积+素数长度无旋转因子内层DFT) — Rader FFT with Winograd Short Convolution for Prime-p Length and Twiddle-Free Inner DFT
 *
 * 功能: 实现Rader FFT算法，针对素数长度使用Winograd短卷积优化，
 *       消除内层DFT的旋转因子乘法，提升素数长度变换效率。
 *
 * 协作: SplitRadixFFT6(FFT) / Goertzel7(Goertzel) / SlidingDFT6(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader FFT(Winograd短卷积+素数长度)
 */
class RaderFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        bool isPrime = false;
        int primitiveRoot = 0;
        int winogradSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT7(QObject *parent = nullptr);
    ~RaderFFT7() override;

    /** @brief Prepare transform for size N (prime or composite) */
    void prepare(int n);

    /** @brief Forward FFT: complex interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse FFT */
    QVector<double> inverse(const QVector<double>& input) const;

    /** @brief Check if n is prime */
    static bool isPrime(int n);

    /** @brief Find primitive root modulo n */
    static int primitiveRoot(int n);

    /** @brief Get prepared size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_n = 0;

    // Rader permutation indices
    QVector<int> m_permA;   // generator powers forward
    QVector<int> m_permB;   // generator powers inverse

    // Winograd convolution kernel (precomputed twiddle DFT)
    QVector<double> m_kernelRe;
    QVector<double> m_kernelIm;

    // For composite sizes: mixed-radix decomposition
    QVector<int> m_factors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Rader permutation from primitive root */
    void buildPermutation(int n, int g);

    /** @brief Precompute Winograd convolution kernel */
    void precomputeKernel();

    /** @brief Circular convolution via FFT */
    void circularConvolve(const QVector<double>& aRe,
                           const QVector<double>& aIm,
                           const QVector<double>& bRe,
                           const QVector<double>& bIm,
                           QVector<double>& outRe,
                           QVector<double>& outIm) const;

    /** @brief Small-N DFT for composite factors */
    void smallDFT(double* re, double* im, int n, int stride, bool inverse) const;

    /** @brief Factorize n into small primes */
    static QVector<int> factorize(int n);

    /** @brief Modular exponentiation */
    static int modPow(int base, int exp, int mod);
};
