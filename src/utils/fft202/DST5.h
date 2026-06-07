/**
 * @file DST5.h
 * @brief 离散正弦变换II型(快速递归+前后旋转因子) — Discrete Sine Transform Type-II via Fast Recursion with Pre-Twiddle and Post-Twiddle Factors
 *
 * 功能: 实现DST-II快速算法，支持递归分解、
 *       前后旋转因子和高精度频域变换。
 *
 * 协作: DCT5(DCT变换) / RaderFFT5(Rader算法) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换II型(快速递归+前后旋转因子)
 */
class DST5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST5(QObject *parent = nullptr);
    ~DST5() override;

    void setTransformSize(int n);

    /** @brief Forward DST-II with normalization */
    void forward(QVector<double>& data);

    /** @brief Inverse DST-II (IDST) with normalization */
    void inverse(QVector<double>& data);

    /** @brief Naive DST-II for verification */
    static QVector<double> naiveDST(const QVector<double>& data);

    /** @brief Precompute sine table and twiddle factors */
    void precompute(int n);

    /** @brief Apply pre-twiddle rotation factors */
    void applyPreTwiddle(QVector<double>& data) const;

    /** @brief Apply post-twiddle rotation factors */
    void applyPostTwiddle(QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_size = 8;

    // Precomputed tables
    QVector<double> m_sinTable;        // sin(pi*(k+1)*(j+1) / (N+1))
    QVector<double> m_preTwiddle;      // pre-twiddle factors
    QVector<double> m_postTwiddle;     // post-twiddle factors
    QVector<double> m_scale;           // normalization scale

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive DST-II core via half-size recursion */
    void dstRecursive(QVector<double>& data, int n) const;

    /** @brief Rearrange input for recursive decomposition */
    void rearrange(QVector<double>& data, int n) const;
};
