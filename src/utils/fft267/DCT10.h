/**
 * @file DCT10.h
 * @brief 离散余弦变换(II型FFT前后预处理与偶扩展正交DCT快速计算) — DCT with Type-II Fast Computation via FFT Pre/Post-Processing and Even-Extension for Orthonormal Discrete Cosine Transform
 *
 * 功能: 实现离散余弦变换(DCT)，采用II型(Type-II)FFT前后预处理(pre/post-processing)
 *       和偶扩展(even-extension)实现正交DCT(orthonormal DCT)快速计算。
 *
 * 协作: BruunFFT10(Bruun FFT) / SplitRadixFFT9(分裂基数FFT) / RDFT8(实数DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换(II型FFT前后预处理与偶扩展正交DCT快速计算)
 */
class DCT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT10(QObject *parent = nullptr);
    ~DCT10() override;

    /** @brief Forward DCT-II (orthonormal) */
    QVector<double> transform(const QVector<double>& input);

    /** @brief Inverse DCT-II (orthonormal, same as DCT-III) */
    QVector<double> inverseTransform(const QVector<double>& spectrum);

    /** @brief Compute DCT-II via FFT pre/post-processing */
    QVector<double> transformViaFFT(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int stages, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& data) const;

    /** @brief In-place radix-2 FFT on interleaved complex data */
    void fftRadix2(QVector<double>& re, QVector<double>& im) const;

    /** @brief Pad to next power of 2 */
    static int nextPow2(int n);

    /** @brief Compute number of stages */
    static int numStages(int n);
};
