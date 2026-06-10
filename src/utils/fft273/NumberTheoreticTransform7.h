/**
 * @file NumberTheoreticTransform7.h
 * @brief 数论变换(Fermat素数模算术与Cooley-Tukey蝶形精确整数卷积) — NTT with Fermat Prime Modular Arithmetic and Cooley-Tukey Butterfly for Exact Integer Convolution
 *
 * 功能: 实现数论变换(NTT)，采用Fermat素数模算术(Fermat prime modular arithmetic)
 *       与Cooley-Tukey蝶形(Cooley-Tukey butterfly)实现精确整数卷积(exact integer convolution)。
 *
 * 协作: SlidingDFT10(滑动DFT) / ZoomFFT7(缩放FFT) / WinogradFFT6(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 数论变换(Fermat素数模算术与Cooley-Tukey蝶形精确整数卷积)
 */
class NumberTheoreticTransform7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoreticTransform7(QObject *parent = nullptr);
    ~NumberTheoreticTransform7() override;

    /** @brief Set transform size (must be power of 2) */
    void setSize(int n);

    /** @brief Set modulus (default: Fermat prime 998244353) */
    void setModulus(qint64 mod);

    /** @brief Set primitive root for modulus */
    void setPrimitiveRoot(qint64 g);

    /** @brief Forward NTT: transforms input in-place */
    QVector<qint64> forward(const QVector<qint64>& input);

    /** @brief Inverse NTT: transforms input in-place */
    QVector<qint64> inverse(const QVector<qint64>& input);

    /** @brief Cyclic convolution of two integer sequences */
    QVector<qint64> convolve(const QVector<qint64>& a, const QVector<qint64>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int size, int numTransforms, double timeMs);

private:
    int m_size = 256;
    qint64 m_mod = 998244353;   // Fermat prime: 119 * 2^23 + 1
    qint64 m_primRoot = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modular exponentiation */
    qint64 modPow(qint64 base, qint64 exp, qint64 mod) const;

    /** @brief Modular inverse via Fermat's little theorem */
    qint64 modInverse(qint64 a, qint64 mod) const;

    /** @brief Bit-reversal permutation index */
    int bitReverse(int x, int logN) const;

    /** @brief Cooley-Tukey butterfly NTT core */
    void butterfly(QVector<qint64>& data, bool inverse);

    /** @brief Precomputed twiddle factors */
    QVector<qint64> m_twiddles;
    QVector<qint64> m_invTwiddles;
    qint64 m_nInverse = 0;

    /** @brief Precompute twiddle factors */
    void precomputeTwiddles();
};
