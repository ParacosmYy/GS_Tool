/**
 * @file WHT10.h
 * @brief 沃尔什-哈达玛变换(Hadamard序快速计算与二进制移位的谱序列分析) — Walsh-Hadamard Transform with Hadamard-ordered Fast Computation and Dyadic Shift for Spectral Sequence Analysis
 *
 * 功能: 实现沃尔什-哈达玛变换(Walsh-Hadamard transform)，采用Hadamard序快速计算(Hadamard-ordered fast computation)
 *       与二进制移位(dyadic shift)实现谱序列分析(spectral sequence analysis)。
 *
 * 协作: FFT10(FFT) / DCT11(DCT-IV) / DST11(DST-IV)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换(Hadamard序快速计算与二进制移位)
 */
class WHT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Transform result */
    struct WHTResult {
        QVector<double> coefficients;
        double totalEnergy = 0.0;
        double dcComponent = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT10(QObject *parent = nullptr);
    ~WHT10() override;

    /** @brief Set transform length N (must be power of 2) */
    void setLength(int n);

    /** @brief Forward WHT in Hadamard order */
    WHTResult transform(const QVector<double>& input);

    /** @brief Inverse WHT (same as forward up to 1/N scaling) */
    QVector<double> inverseTransform(const QVector<double>& coeffs);

    /** @brief Apply dyadic shift (XOR shift) in sequency domain */
    QVector<double> dyadicShift(const QVector<double>& coeffs, int shift) const;

    /** @brief Compute Walsh-ordered (sequency-ordered) coefficients */
    QVector<double> toSequencyOrder(const QVector<double>& hadamardCoeffs) const;

    /** @brief Compute power spectrum from WHT coefficients */
    QVector<double> powerSpectrum(const QVector<double>& coeffs) const;

    /** @brief Filter coefficients by threshold, reconstruct signal */
    QVector<double> thresholdFilter(const QVector<double>& input, double thresholdRatio);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, double energy, double timeMs);

private:
    int m_N = 16;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed bit-reversal permutation for sequency ordering */
    QVector<int> m_bitReversal;

    /** @brief Fast WHT core via butterfly operations */
    void fastWHT(QVector<double>& data) const;

    /** @brief Compute bit-reversal permutation for sequency order */
    void computeBitReversal();

    /** @brief Check if n is power of 2 */
    static bool isPowerOf2(int n);
};
