/**
 * @file DST7.h
 * @brief IV型DST(对称扩展+前预扭DCT快速计算管线) — Type-IV DST with Symmetric Extension and Fast Computation via Pre-Twiddle DCT-Based Pipeline
 *
 * 功能: 实现Type-IV离散正弦变换(DST-IV)，通过对称扩展和前预扭DCT管线
 *       实现快速O(n log n)计算，避免直接矩阵乘法。
 *
 * 协作: DCT7(IV型DCT) / FFTCore5(FFT核心) / MDCT5(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief IV型DST(对称扩展+前预扭DCT管线)
 */
class DST7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        int numTwiddles = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST7(QObject *parent = nullptr);
    ~DST7() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n);

    /** @brief Forward DST-IV transform */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse DST-IV (self-inverse up to normalization) */
    QVector<double> inverse(const QVector<double>& input) const;

    /** @brief Get prepared size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_n = 0;
    int m_stages = 0;

    // Pre-twiddle factors for DCT-based pipeline
    QVector<double> m_preTwiddleCos;
    QVector<double> m_preTwiddleSin;

    // Post-twiddle factors
    QVector<double> m_postTwiddleCos;
    QVector<double> m_postTwiddleSin;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pre-twiddle factors */
    void computePreTwiddles();

    /** @brief Compute post-twiddle factors */
    void computePostTwiddles();

    /** @brief Symmetric extension of input for DCT conversion */
    QVector<double> symmetricExtend(const QVector<double>& input) const;

    /** @brief In-place bit-reversal permutation */
    void bitReverse(QVector<double>& data) const;

    /** @brief DCT-II butterfly computation */
    void dct2Butterfly(QVector<double>& data) const;

    /** @brief Apply pre-twiddle rotation */
    void applyPreTwiddle(QVector<double>& data) const;

    /** @brief Apply post-twiddle rotation */
    void applyPostTwiddle(QVector<double>& data) const;
};
