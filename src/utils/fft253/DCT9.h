/**
 * @file DCT9.h
 * @brief 离散余弦变换(快速Type-IV+移位DCT-II分解+正交归一化) — DCT with Fast Type-IV Computation via Shifted-DCT-II Decomposition and Orthogonal Normalization Factors
 *
 * 功能: 实现离散余弦变换(DCT)，支持快速Type-IV计算(fast Type-IV)
 *       通过移位DCT-II分解(shifted-DCT-II decomposition)降低计算复杂度，
 *       正交归一化因子(orthogonal normalization factors)保证变换正交性。
 *
 * 协作: BruunFFT9(Bruun FFT) / PrimeFactorFFT9(素因子FFT) / MDCT7(改进DCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换(Type-IV+移位DCT-II分解)
 */
class DCT9 : public QObject {
    Q_OBJECT

public:
    /** @brief DCT type selector */
    enum Type { TypeII = 2, TypeIII = 3, TypeIV = 4 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numTypeIV = 0;
        int butterflyOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT9(QObject *parent = nullptr);
    ~DCT9() override;

    /** @brief Prepare transform for size N */
    bool prepare(int n, Type type = TypeII);

    /** @brief Forward DCT transform */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse DCT transform */
    QVector<double> inverse(const QVector<double>& coefficients);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int type, double timeMs);

private:
    int m_size = 0;
    Type m_type = TypeII;
    QVector<double> m_cosTable;    // Precomputed cosines
    QVector<double> m_normFactors; // Orthogonal normalization

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute cosine table and normalization factors */
    void precompute();

    /** @brief DCT-II implementation */
    QVector<double> dctII(const QVector<double>& x) const;

    /** @brief DCT-III (inverse of DCT-II) */
    QVector<double> dctIII(const QVector<double>& x) const;

    /** @brief DCT-IV via shifted DCT-II decomposition */
    QVector<double> dctIV(const QVector<double>& x) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& data) const;

    /** @brief Check if n is power of 2 */
    static bool isPowerOf2(int n);
};
