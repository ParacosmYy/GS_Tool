/**
 * @file NumberTheoreticTransform5.h
 * @brief 数论变换(Barrett约减模算术+Bluestein扩展支持任意复合长度) — NTT with Barrett Reduction Modular Arithmetic and Bluestein Extension for Arbitrary Composite Lengths
 *
 * 功能: 实现数论变换(Number Theoretic Transform)，使用Barrett约减(Barrett
 *       reduction)加速模算术运算，通过Bluestein扩展(Bluestein extension)支持
 *       任意复合长度(arbitrary composite lengths)的变换。
 *
 * 协作: SlidingDFT8(滑动DFT) / ZoomFFT5(缩放FFT) / BruunFFT8(Bruun FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Barrett约减模算术+Bluestein扩展支持任意复合长度)
 */
class NumberTheoreticTransform5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        quint64 barrettReductions = 0;
        bool usedBluestein = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform5(QObject *parent = nullptr);
    ~NumberTheoreticTransform5() override;

    /** @brief Set modulus (must be prime of form k*2^m+1) */
    void setModulus(quint64 mod);

    /** @brief Set primitive root of unity */
    void setPrimitiveRoot(quint64 g);

    /** @brief Forward NTT (in-place on QVector) */
    QVector<quint64> forward(const QVector<quint64>& input);

    /** @brief Inverse NTT */
    QVector<quint64> inverse(const QVector<quint64>& input);

    /** @brief Polynomial multiplication via NTT */
    QVector<quint64> multiply(const QVector<quint64>& a, const QVector<quint64>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool usedBluestein, double timeMs);

private:
    quint64 m_mod = 998244353;       // Default prime: 119 * 2^23 + 1
    quint64 m_primRoot = 3;          // Default primitive root
    quint64 m_barrettK = 0;          // Barrett precomputed constant

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Barrett reduction: compute a % m efficiently */
    quint64 barrettReduce(quint64 a) const;

    /** @brief Modular multiplication with Barrett reduction */
    quint64 modMul(quint64 a, quint64 b) const;

    /** @brief Modular exponentiation */
    quint64 modPow(quint64 base, quint64 exp) const;

    /** @brief Precompute Barrett constant */
    void precomputeBarrett();

    /** @brief Bit-reverse permutation */
    void bitReverse(QVector<quint64>& data) const;

    /** @brief Power-of-two NTT (Cooley-Tukey butterfly) */
    void nttCT(QVector<quint64>& data, bool inverse);

    /** @brief Bluestein's algorithm for arbitrary-length NTT */
    QVector<quint64> bluestein(const QVector<quint64>& input, bool inverse);

    /** @brief Next power of two >= n */
    static int nextPow2(int n);

    /** @brief Find n-th root of unity mod m */
    quint64 findRoot(int n) const;
};
