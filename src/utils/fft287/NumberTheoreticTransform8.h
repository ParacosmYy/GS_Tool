/**
 * @file NumberTheoreticTransform8.h
 * @brief 数论变换(Montgomery乘法与Barrett约减大整数卷积) — NTT with Montgomery Multiplication and Barrett Reduction for Efficient Modular Arithmetic in Large Integer Convolution
 *
 * 功能: 实现数论变换(NTT)，采用Montgomery乘法(Montgomery multiplication)
 *       与Barrett约减(Barrett reduction)实现高效模算术大整数卷积(large integer convolution)。
 *
 * 协作: SlidingDFT11(滑动DFT) / FFT10(FFT) / ZoomFFT8(缩放FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Montgomery乘法与Barrett约减大整数卷积)
 */
class NumberTheoreticTransform8 : public QObject {
    Q_OBJECT

public:
    /** @brief NTT configuration */
    struct NTTConfig {
        quint64 modulus = 998244353;      // Prime modulus (NTT-friendly)
        quint64 primitiveRoot = 3;        // Primitive root of modulus
        int transformSize = 0;            // 0 = auto from input
    };

    /** @brief Convolution result */
    struct ConvolutionResult {
        QVector<quint64> result;
        int transformSize = 0;
        int actualLength = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastTransformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform8(QObject *parent = nullptr);
    ~NumberTheoreticTransform8() override;

    void setConfig(const NTTConfig& cfg);

    /** @brief Forward NTT (in-place) */
    void forwardNTT(QVector<quint64>& data);

    /** @brief Inverse NTT (in-place) */
    void inverseNTT(QVector<quint64>& data);

    /** @brief Polynomial multiplication via NTT convolution */
    ConvolutionResult multiply(const QVector<quint64>& a, const QVector<quint64>& b);

    /** @brief Cyclic convolution */
    QVector<quint64> cyclicConvolve(const QVector<quint64>& a, const QVector<quint64>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nttDone(int size, double timeMs);
    void convolutionDone(int len, double timeMs);

private:
    NTTConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Montgomery parameters
    quint64 m_montR = 0;       // R = 2^k mod modulus
    quint64 m_montRInv = 0;    // R^-1 mod modulus
    quint64 m_montNPrime = 0;  // -modulus^-1 mod R

    // Barrett parameters
    quint64 m_barrettK = 0;
    quint64 m_barrettMu = 0;

    /** @brief Compute next power of 2 */
    static int nextPow2(int n);

    /** @brief Montgomery modular multiplication */
    quint64 montMul(quint64 a, quint64 b) const;

    /** @brief Barrett reduction: a mod modulus */
    quint64 barrettReduce(quint64 a) const;

    /** @brief Modular addition */
    quint64 modAdd(quint64 a, quint64 b) const;

    /** @brief Modular subtraction */
    quint64 modSub(quint64 a, quint64 b) const;

    /** @brief Fast modular exponentiation */
    quint64 modPow(quint64 base, quint64 exp) const;

    /** @brief Compute n-th root of unity */
    quint64 rootOfUnity(int n) const;

    /** @brief Initialize Montgomery and Barrett parameters */
    void initModParams();
};
