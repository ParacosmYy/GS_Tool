/**
 * @file NumberTheoreticTransform2.h
 * @brief 数论变换(有限域NTT+原根+CRT大整数卷积) — Number Theoretic Transform over Finite Field with Primitive Root Finding and CRT-Based Large-Integer Convolution
 *
 * 功能: 实现数论变换，支持有限域上NTT、原根自动寻找、
 *       模逆运算和CRT大整数多项式卷积。
 *
 * 协作: WHT4(沃尔什变换) / FFT2(快速傅里叶) / FNT3(费马数变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换器(有限域NTT+CRT大整数卷积)
 */
class NumberTheoreticTransform2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        qint64 modulus = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform2(QObject *parent = nullptr);
    ~NumberTheoreticTransform2() override;

    void setModulus(qint64 mod);
    void setPrimitiveRoot(qint64 root);

    /** @brief 正向NTT变换 */
    QVector<qint64> forward(const QVector<qint64>& input);

    /** @brief 逆向NTT变换 */
    QVector<qint64> inverse(const QVector<qint64>& input);

    /** @brief 多项式乘法(NTT卷积) */
    QVector<qint64> multiply(const QVector<qint64>& a,
                             const QVector<qint64>& b);

    /** @brief CRT合并两个模数的卷积结果 */
    QVector<qint64> crtMerge(const QVector<qint64>& r1, qint64 m1,
                             const QVector<qint64>& r2, qint64 m2) const;

    /** @brief 寻找模数的原根 */
    qint64 findPrimitiveRoot(qint64 mod) const;

    /** @brief 模幂运算 */
    qint64 modPow(qint64 base, qint64 exp, qint64 mod) const;

    /** @brief 模逆(扩展欧几里得) */
    qint64 modInverse(qint64 a, qint64 mod) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, qint64 modulus, double timeMs);

private:
    qint64 m_modulus = 998244353;      // Popular NTT-friendly prime
    qint64 m_primitiveRoot = 3;        // Primitive root of 998244353

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<qint64>& data) const;

    /** @brief In-place NTT butterfly */
    void nttButterfly(QVector<qint64>& data, bool invert);

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Check if n is prime (Miller-Rabin) */
    bool isPrime(qint64 n) const;
};
