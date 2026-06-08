/**
 * @file WHT6.h
 * @brief Walsh-Hadamard变换(自然序快速Hadamard+Gray码位反转列率转换) — Walsh-Hadamard Transform with Natural-Order Fast Hadamard and Sequency Conversion via Gray Code Bit-Reversal
 *
 * 功能: 实现Walsh-Hadamard变换的自然序快速Hadamard算法(FHT)，
 *       支持通过Gray码位反转换序实现列率(sequency)排序。
 *
 * 协作: FFTCore5(FFT核心) / DST7(IV型DST) / DWT8(离散小波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Walsh-Hadamard变换(自然序FHT+Gray码列率转换)
 */
class WHT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Transform ordering mode */
    enum Ordering {
        NaturalOrder,   // natural (Hadamard) order
        SequencyOrder   // Walsh (sequency) order via Gray code
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        int numForwards = 0;
        int numInverses = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT6(QObject *parent = nullptr);
    ~WHT6() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n);

    /** @brief Forward WHT in specified ordering */
    QVector<double> forward(const QVector<double>& input,
                            Ordering order = SequencyOrder) const;

    /** @brief Inverse WHT (self-inverse up to normalization) */
    QVector<double> inverse(const QVector<double>& input,
                            Ordering order = SequencyOrder) const;

    /** @brief Convert natural-order to sequency-order via Gray code */
    QVector<double> toSequencyOrder(const QVector<double>& natural) const;

    /** @brief Convert sequency-order to natural-order */
    QVector<double> toNaturalOrder(const QVector<double>& sequency) const;

    /** @brief Compute sequency (zero-crossing count) of each basis */
    QVector<int> sequencyTable() const;

    int size() const { return m_n; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, const QString& order, double timeMs);

private:
    int m_n = 0;
    int m_stages = 0;

    // Gray code permutation index for sequency ordering
    QVector<int> m_grayPermute;
    // Inverse permutation
    QVector<int> m_grayPermuteInv;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Gray code bit-reversal permutation */
    void computeGrayPermutation();

    /** @brief In-place natural-order fast Hadamard transform */
    void fastHadamard(QVector<double>& data) const;

    /** @brief Bit-reversal of an integer */
    int bitReverse(int val, int bits) const;

    /** @brief Compute Gray code of value */
    int grayCode(int val) const;
};
