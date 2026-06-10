/**
 * @file DST10.h
 * @brief 离散正弦变换(II型DFT嵌入与奇对称扩展正交DST快速计算) — DST with Type-II Fast Computation via DFT Embedding and Odd-Symmetry Extension for Orthonormal Discrete Sine Transform
 *
 * 功能: 实现离散正弦变换(DST)，采用II型(Type-II)DFT嵌入(DFT embedding)
 *       和奇对称扩展(odd-symmetry extension)实现正交DST(orthonormal DST)快速计算。
 *
 * 协作: DCT10(离散余弦变换) / BruunFFT10(Bruun FFT) / SplitRadixFFT9(分裂基数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换(II型DFT嵌入与奇对称扩展正交DST快速计算)
 */
class DST10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST10(QObject *parent = nullptr);
    ~DST10() override;

    /** @brief Forward DST-II (orthonormal) */
    QVector<double> transform(const QVector<double>& input);

    /** @brief Inverse DST-II (orthonormal, same as DST-III) */
    QVector<double> inverseTransform(const QVector<double>& spectrum);

    /** @brief Compute DST-II via DFT embedding with odd extension */
    QVector<double> transformViaDFT(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int stages, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place radix-2 FFT */
    void fftRadix2(QVector<double>& re, QVector<double>& im) const;

    /** @brief Pad to next power of 2 */
    static int nextPow2(int n);

    /** @brief Compute number of stages */
    static int numStages(int n);
};
