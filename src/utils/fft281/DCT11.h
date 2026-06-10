/**
 * @file DCT11.h
 * @brief 离散余弦变换IV型(半移与特征值分解的改进离散余弦变换) — DCT with Type-IV Computation via Half-shift and Eigenvalue Decomposition for Modified Discrete Cosine Transform
 *
 * 功能: 实现离散余弦变换IV型(DCT-IV)，采用半移(half-shift)
 *       与特征值分解(eigenvalue decomposition)实现改进离散余弦变换(modified discrete cosine transform)。
 *
 * 协作: BruunFFT11(Bruun FFT) / MDCT10(MDCT) / DFT12(DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换IV型(半移与特征值分解)
 */
class DCT11 : public QObject {
    Q_OBJECT

public:
    /** @brief Transform result */
    struct DCTResult {
        QVector<double> coefficients;
        double energy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT11(QObject *parent = nullptr);
    ~DCT11() override;

    /** @brief Set transform length N */
    void setLength(int n);

    /** @brief Forward DCT-IV */
    DCTResult transform(const QVector<double>& input);

    /** @brief Inverse DCT-IV (same as forward for type-IV, with 2/N scaling) */
    QVector<double> inverseTransform(const QVector<double>& coeffs);

    /** @brief Forward MDCT (modified DCT) using DCT-IV core */
    DCTResult mdctForward(const QVector<double>& input);

    /** @brief Inverse MDCT using overlap-add with DCT-IV */
    QVector<double> mdctInverse(const QVector<double>& coeffs, const QVector<double>& prevTail);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, double energy, double timeMs);

private:
    int m_N = 64;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed cosine table for DCT-IV twiddle factors */
    QVector<double> m_cosTable;

    /** @brief Precompute cosine table */
    void precomputeCosines();

    /** @brief Reorder input with half-shift for DCT-IV */
    QVector<double> halfShift(const QVector<double>& input) const;

    /** @brief Compute eigenvalue-based correction for modified DCT */
    void eigenvalueCorrection(QVector<double>& data) const;

    /** @brief Apply 2N-point FFT-based core computation */
    void fftBasedDCT4(QVector<double>& data) const;
};
