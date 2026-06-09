/**
 * @file RaderFFT9.h
 * @brief Rader FFT(Winograd短卷积模块素数长度+旋转因子预计算缓存) — Rader FFT with Winograd Short Convolution Modules for Prime-p Lengths and Twiddle Factor Precomputation Caching
 *
 * 功能: 实现Rader FFT算法(Rader's FFT Algorithm)，针对素数长度(p)的DFT
 *       通过原根置换转化为循环卷积，使用Winograd短卷积模块(Winograd short
 *       convolution modules)加速计算，旋转因子(twiddle factor)预计算缓存。
 *
 * 协作: Goertzel9(Goertzel算法) / SplitRadixFFT8(分裂基FFT) / BluesteinFFT8(Bluestein FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader FFT(Winograd短卷积+旋转因子缓存)
 */
class RaderFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int cacheHits = 0;
        int cacheMisses = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT9(QObject *parent = nullptr);
    ~RaderFFT9() override;

    /** @brief Precompute Rader plan for a given prime size */
    void prepare(int primeSize);

    /** @brief Forward FFT (complex interleaved: [re0,im0,re1,im1,...]) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Check if N is prime */
    static bool isPrime(int n);

    /** @brief Find primitive root modulo p */
    static int primitiveRoot(int p);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool forward, double timeMs);

private:
    int m_size = 0;

    /** @brief Precomputed Rader plan */
    struct RaderPlan {
        int p = 0;
        int g = 0;                        // Primitive root
        QVector<int> perm;                // Permutation indices
        QVector<double> twiddleReal;      // Precomputed twiddle factors
        QVector<double> twiddleImag;
        QVector<double> convKernelReal;   // Circular convolution kernel
        QVector<double> convKernelImag;
    };

    RaderPlan m_plan;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Rader convolution kernel */
    void buildKernel();

    /** @brief Circular convolution via direct method (p-1 length) */
    void circularConvolve(const QVector<double>& ar, const QVector<double>& ai,
                           const QVector<double>& br, const QVector<double>& bi,
                           QVector<double>& outR, QVector<double>& outI) const;

    /** @brief Modular exponentiation */
    static int modPow(int base, int exp, int mod);
};
