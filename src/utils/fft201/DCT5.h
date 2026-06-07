/**
 * @file DCT5.h
 * @brief 离散余弦变换II型(快速递归+正交前后乘) — Discrete Cosine Transform Type-II via Fast Recursion with Pre/Post Multiplication for Orthogonality
 *
 * 功能: 实现DCT-II快速算法，支持递归分解、
 *       正交归一化前后乘因子和高精度频域变换。
 *
 * 协作: RaderFFT5(Rader算法) / FFTW5(FFT引擎) / MDCT3(MDCT变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换II型(快速递归+正交前后乘)
 */
class DCT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT5(QObject *parent = nullptr);
    ~DCT5() override;

    void setTransformSize(int n);

    /** @brief Forward DCT-II with orthogonal scaling */
    void forward(QVector<double>& data);

    /** @brief Inverse DCT-II (IDCT) with orthogonal scaling */
    void inverse(QVector<double>& data);

    /** @brief Naive DCT-II for verification */
    static QVector<double> naiveDCT(const QVector<double>& data);

    /** @brief Precompute twiddle factors for size n */
    void precompute(int n);

    /** @brief Apply pre-multiplication for orthogonal normalization */
    void applyPreMultiply(QVector<double>& data) const;

    /** @brief Apply post-multiplication for orthogonal normalization */
    void applyPostMultiply(QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_size = 8;

    // Precomputed tables
    QVector<double> m_cosTable;       // cos(pi*(2k+1)*j / 2N)
    QVector<double> m_preScale;       // pre-multiply factors
    QVector<double> m_postScale;      // post-multiply factors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive DCT-II core via half-size DCT */
    void dctRecursive(QVector<double>& data, int n) const;

    /** @brief Rearrange input for recursive decomposition */
    void rearrange(QVector<double>& data, int n) const;
};
