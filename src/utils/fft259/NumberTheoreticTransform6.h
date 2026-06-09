/**
 * @file NumberTheoreticTransform6.h
 * @brief 数论变换(Cooley-Tukey蝶形运算+NTT多项式乘法) — NTT with Cooley-Tukey Butterfly for Power-of-two Moduli and NTT-based Polynomial Multiplication
 *
 * 功能: 实现数论变换(NTT)，采用Cooley-Tukey蝶形运算(Cooley-Tukey
 *       butterfly)处理二的幂次模数，支持NTT多项式乘法(NTT-based
 *       polynomial multiplication)实现高效大整数卷积。
 *
 * 协作: SlidingDFT9(滑动DFT) / FFT4(FFT) / ZoomFFT6(缩放FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Cooley-Tukey蝶形运算+NTT多项式乘法)
 */
class NumberTheoreticTransform6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numConvolutions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform6(QObject *parent = nullptr);
    ~NumberTheoreticTransform6() override;

    /** @brief Set NTT modulus (must be prime of form k*2^n+1) */
    void setModulus(quint64 mod);

    /** @brief Set primitive root */
    void setPrimitiveRoot(quint64 root);

    /** @brief Forward NTT (in-place) */
    void forward(QVector<quint64>& data);

    /** @brief Inverse NTT (in-place) */
    void inverse(QVector<quint64>& data);

    /** @brief NTT-based polynomial multiplication */
    QVector<quint64> multiply(const QVector<quint64>& a, const QVector<quint64>& b);

    /** @brief Point-wise multiply two NTT-transformed vectors */
    QVector<quint64> pointwiseMultiply(const QVector<quint64>& a,
                                        const QVector<quint64>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool inverse, double timeMs);
    void convolutionCompleted(int resultSize, double timeMs);

private:
    quint64 m_mod = 998244353;    // Default NTT-friendly prime
    quint64 m_root = 3;           // Primitive root of m_mod
    int m_logN = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modular exponentiation */
    quint64 modPow(quint64 base, quint64 exp, quint64 mod) const;

    /** @brief Modular inverse */
    quint64 modInverse(quint64 a, quint64 mod) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<quint64>& data) const;

    /** @brief Cooley-Tukey butterfly NTT core */
    void butterflyNTT(QVector<quint64>& data, bool inverse);

    /** @brief Next power of two */
    int nextPow2(int n) const;
};
