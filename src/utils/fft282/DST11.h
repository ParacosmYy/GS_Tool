/**
 * @file DST11.h
 * @brief 离散正弦变换IV型(DCT-IV关系的奇对称修正离散正弦变换) — DST with Type-IV Computation via DCT-IV Relationship and Odd-symmetry for Modified Discrete Sine Transform
 *
 * 功能: 实现离散正弦变换IV型(DST-IV)，采用DCT-IV关系(DCT-IV relationship)
 *       与奇对称(odd-symmetry)实现修正离散正弦变换(modified discrete sine transform)。
 *
 * 协作: DCT11(DCT-IV) / FFT10(FFT) / MDCT10(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换IV型(DCT-IV关系的奇对称)
 */
class DST11 : public QObject {
    Q_OBJECT

public:
    /** @brief Transform result */
    struct DSTResult {
        QVector<double> coefficients;
        double energy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST11(QObject *parent = nullptr);
    ~DST11() override;

    /** @brief Set transform length N */
    void setLength(int n);

    /** @brief Forward DST-IV via DCT-IV relationship */
    DSTResult transform(const QVector<double>& input);

    /** @brief Inverse DST-IV (self-inverse up to 2/N scaling) */
    QVector<double> inverseTransform(const QVector<double>& coeffs);

    /** @brief Forward MDST using DST-IV core */
    DSTResult mdstForward(const QVector<double>& input);

    /** @brief Inverse MDST using overlap-add */
    QVector<double> mdstInverse(const QVector<double>& coeffs, const QVector<double>& prevTail);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, double energy, double timeMs);

private:
    int m_N = 64;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed sine table for DST-IV twiddle factors */
    QVector<double> m_sinTable;

    /** @brief Precompute sine table */
    void precomputeSines();

    /** @brief Compute DST-IV via DCT-IV with reordering */
    void dst4Core(QVector<double>& data) const;

    /** @brief Apply odd-symmetry reordering for DST-IV <-> DCT-IV */
    QVector<double> oddSymmetryReorder(const QVector<double>& input) const;

    /** @brief Reverse odd-symmetry reordering */
    QVector<double> oddSymmetryInverse(const QVector<double>& data) const;
};
