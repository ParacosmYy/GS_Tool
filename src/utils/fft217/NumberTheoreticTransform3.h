/**
 * @file NumberTheoreticTransform3.h
 * @brief 数论变换(Barrett约减模运算+Fermat数变换支持2的幂长度) — Number Theoretic Transform with Barrett Reduction Modulus Arithmetic and Fermat Number Transform for Power-of-Two Lengths
 *
 * 功能: 实现数论变换NTT，使用Barrett约减加速模运算，
 *       支持Fermat数变换处理2的幂长度序列。
 *
 * 协作: SlidingDFT6(滑动DFT) / DistributedArithmetic6(分布式算术) / WinogradFFT4(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Barrett约减+Fermat数变换)
 */
class NumberTheoreticTransform3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        quint64 modulus = 0;
        quint64 primitiveRoot = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform3(QObject *parent = nullptr);
    ~NumberTheoreticTransform3() override;

    /** @brief Set transform parameters: size, modulus, primitive root */
    void setParameters(int size, quint64 modulus = 0, quint64 primitiveRoot = 0);

    /** @brief Forward NTT */
    QVector<quint64> forward(const QVector<quint64>& input);

    /** @brief Inverse NTT */
    QVector<quint64> inverse(const QVector<quint64>& input);

    /** @brief Polynomial multiplication via NTT */
    QVector<quint64> multiply(const QVector<quint64>& a,
                               const QVector<quint64>& b);

    /** @brief Barrett reduction: compute x mod m efficiently */
    quint64 barrettReduce(quint64 x, quint64 m, quint64 factor) const;

    /** @brief Modular exponentiation */
    quint64 modPow(quint64 base, quint64 exp, quint64 m) const;

    /** @brief Compute modular inverse via Fermat's little theorem */
    quint64 modInverse(quint64 a, quint64 m) const;

    /** @brief Find primitive root for given modulus */
    quint64 findPrimitiveRoot(quint64 m) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, quint64 modulus, double timeMs);

private:
    int m_size = 256;
    quint64 m_modulus = 998244353;      // NTT-friendly prime
    quint64 m_primitiveRoot = 3;
    quint64 m_barrettFactor = 0;        // Precomputed for Barrett reduction

    // Precomputed tables
    QVector<quint64> m_bitRev;
    QVector<quint64> m_rootPowers;
    QVector<quint64> m_invRootPowers;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute bit-reversal permutation */
    void computeBitRev();

    /** @brief Precompute root power tables */
    void computeRootTables();

    /** @brief Precompute Barrett reduction factor */
    void computeBarrettFactor();

    /** @brief Core NTT butterfly with Barrett reduction */
    void butterfly(QVector<quint64>& data, bool inverse);
};
