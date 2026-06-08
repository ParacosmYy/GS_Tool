/**
 * @file NumberTheoreticTransform4.h
 * @brief 数论变换(Montgomery乘法+Solinas素数模快速约减) — NTT with Montgomery Multiplication and Prime Modulus Selection via Solinas Form for Fast Modular Reduction
 *
 * 功能: 实现数论变换(Number Theoretic Transform)，采用Montgomery乘法加速模运算，
 *       选择Solinas形式的素数模(Solinas prime)以实现快速模约减(modular reduction)。
 *
 * 协作: FFTCore5(FFT核心) / SlidingDFT7(滑动DFT) / Goertzel6(Goertzel)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Montgomery乘法+Solinas素数模快速约减)
 */
class NumberTheoreticTransform4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        quint64 modulus = 0;
        quint64 primitiveRoot = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform4(QObject *parent = nullptr);
    ~NumberTheoreticTransform4() override;

    /** @brief Configure NTT size and optional modulus (0 = auto-select Solinas prime) */
    bool configure(int n, quint64 modulus = 0);

    /** @brief Forward NTT (in-place) */
    void forward(QVector<qint64>& data);

    /** @brief Inverse NTT (in-place) */
    void inverse(QVector<qint64>& data);

    /** @brief Pointwise multiply two NTT-domain vectors */
    QVector<qint64> pointwiseMultiply(const QVector<qint64>& a,
                                       const QVector<qint64>& b) const;

    /** @brief Polynomial multiply via NTT: result = a * b */
    QVector<qint64> polynomialMultiply(const QVector<qint64>& a,
                                        const QVector<qint64>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, double timeMs);

private:
    int m_n = 0;
    int m_logN = 0;
    quint64 m_mod = 0;         // Solinas prime
    quint64 m_root = 0;        // primitive root of unity
    quint64 m_rootInv = 0;     // inverse root

    // Montgomery reduction parameters
    quint64 m_montR = 0;       // R = 2^64 mod m_mod
    quint64 m_montR2 = 0;      // R^2 mod m_mod
    quint64 m_montNPrime = 0;  // -mod^(-1) mod R

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Select Solinas prime of form 2^k - 2^j + 1 suitable for size n */
    quint64 selectSolinasPrime(int n) const;

    /** @brief Find primitive root of order n modulo prime */
    quint64 findPrimitiveRoot(int n, quint64 mod) const;

    /** @brief Montgomery modular multiplication: a*b mod m */
    quint64 montMul(quint64 a, quint64 b) const;

    /** @brief Convert to Montgomery form */
    quint64 toMont(quint64 x) const;

    /** @brief Convert from Montgomery form */
    quint64 fromMont(quint64 x) const;

    /** @brief Modular exponentiation using Montgomery multiplication */
    quint64 montPow(quint64 base, quint64 exp) const;

    /** @brief Modular inverse via Fermat's little theorem */
    quint64 modInverse(quint64 a) const;

    /** @brief Solinas-form fast modular reduction */
    quint64 solinasReduce(quint64 x) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<qint64>& data) const;
};
