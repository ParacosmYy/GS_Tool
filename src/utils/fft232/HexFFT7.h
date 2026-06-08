/**
 * @file HexFFT7.h
 * @brief 六角FFT(radix-3/6混合分解+旋转因子共享) — Hex FFT with Radix-3/6 Hybrid Decomposition and Twiddle Factor Sharing for Composite 6k-Length Transforms
 *
 * 功能: 实现六角FFT(Hex FFT)算法，采用radix-3/6混合分解策略，
 *       通过旋转因子共享(twiddle factor sharing)优化复合6k长度变换的计算效率。
 *
 * 协作: FFTCore5(FFT核心) / NumberTheoreticTransform4(数论变换) / SlidingDFT7(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六角FFT(radix-3/6混合分解+旋转因子共享)
 */
class HexFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numRadix3 = 0;
        int numRadix6 = 0;
        int numTwiddleShared = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT7(QObject *parent = nullptr);
    ~HexFFT7() override;

    /** @brief Configure transform size (must be composite 6k, i.e. divisible by 6) */
    bool configure(int n);

    /** @brief Forward transform (complex interleaved: real0,imag0,real1,imag1,...) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse transform (complex interleaved) */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief Get the factorization plan for current N */
    QVector<int> factorizationPlan() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, double timeMs);

private:
    int m_n = 0;
    QVector<int> m_factors;  // decomposition factors (3 and 6)

    // Pre-computed twiddle factors for sharing
    QVector<double> m_twiddleReal;
    QVector<double> m_twiddleImag;
    // Shared twiddle bank for radix-3 and radix-6 overlap
    QVector<double> m_sharedTwiddleReal;
    QVector<double> m_sharedTwiddleImag;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into 3s and 6s */
    QVector<int> factorize6k(int n) const;

    /** @brief Pre-compute all twiddle factors with sharing */
    void computeTwiddleFactors();

    /** @brief Radix-3 butterfly */
    void radix3Butterfly(double* re, double* im, int stride, int m,
                          const double* twRe, const double* twIm) const;

    /** @brief Radix-6 butterfly (built from radix-3 + radix-2) */
    void radix6Butterfly(double* re, double* im, int stride, int m,
                          const double* twRe, const double* twIm) const;

    /** @brief General Cooley-Tukey stage for factor f */
    void ctStage(double* re, double* im, int n, int factor, int stride, int twOff);

    /** @brief Digit-reverse permutation for mixed radix */
    void digitReverse(double* re, double* im) const;

    /** @brief Compute complex exponential: cos and -sin */
    static void twiddle(double angle, double& cosVal, double& sinVal);
};
