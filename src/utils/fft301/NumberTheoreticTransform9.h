/**
 * @file NumberTheoreticTransform9.h
 * @brief 数论变换(Cooley-Tukey蝶形分解与盲化因子实现零知识证明兼容数论变换) — NTT with Cooley-Tukey Butterfly Decomposition and Blinding Factor for Zero-Knowledge Proof Compatible Number Theoretic Transform
 *
 * 功能: 实现数论变换(NTT)，采用Cooley-Tukey蝶形分解(Cooley-Tukey butterfly decomposition)
 *       与盲化因子(blinding factor)实现零知识证明兼容数论变换(ZKP-compatible number theoretic transform)。
 *
 * 协作: SplitRadixFFT11(分裂基FFT) / SlidingDFT12(滑动DFT) / ModularArithmetic(模算术)
 */
#pragma once

#include <QObject>
#include <QVector>

class NumberTheoreticTransform9 : public QObject {
    Q_OBJECT

public:
    /** @brief NTT parameters */
    struct NTTParams {
        qint64 modulus = 998244353;      // prime modulus
        qint64 primitiveRoot = 3;        // primitive root of unity
        int logSize = 0;                 // log2(transform size)
    };

    /** @brief Transform result */
    struct TransformResult {
        QVector<qint64> values;
        int size = 0;
        qint64 modulus = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        qint64 modulus = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform9(QObject *parent = nullptr);
    ~NumberTheoreticTransform9() override;

    void setModulus(qint64 mod);
    void setPrimitiveRoot(qint64 root);
    void setBlindingEnabled(bool enabled);

    /** @brief Forward NTT with Cooley-Tukey butterfly */
    TransformResult forward(const QVector<qint64>& input);

    /** @brief Inverse NTT */
    TransformResult inverse(const QVector<qint64>& input);

    /** @brief Pointwise multiply two NTT-domain vectors */
    QVector<qint64> pointwiseMultiply(const QVector<qint64>& a, const QVector<qint64>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int size, qint64 modulus, double timeMs);

private:
    qint64 m_modulus = 998244353;
    qint64 m_primitiveRoot = 3;
    bool m_blinding = false;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modular exponentiation */
    qint64 modPow(qint64 base, qint64 exp, qint64 mod) const;

    /** @brief Modular inverse via Fermat's little theorem */
    qint64 modInverse(qint64 a, qint64 mod) const;

    /** @brief Find n-th root of unity mod p */
    qint64 findRootOfUnity(int n) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<qint64>& data) const;

    /** @brief Generate blinding scalar for ZKP masking */
    qint64 generateBlindingFactor() const;

    /** @brief Apply blinding to input vector */
    void applyBlinding(QVector<qint64>& data) const;

    /** @brief Core Cooley-Tukey butterfly NTT */
    void butterflyNTT(QVector<qint64>& data, bool inverse);
};
