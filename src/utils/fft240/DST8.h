/**
 * @file DST8.h
 * @brief 离散正弦变换(I型快速计算+FFT奇对称扩展+高效正弦变换) — DST with Type-I Fast Computation via FFT and Odd-Symmetry Extension for Efficient Sine Transform
 *
 * 功能: 实现离散正弦变换(DST)，采用I型快速计算(type-I fast computation)通过FFT奇对称扩展
 *       (FFT odd-symmetry extension)将DST转换为等效FFT计算，利用奇对称性(odd-symmetry)
 *       消除冗余计算，实现高效正弦变换。
 *
 * 协作: DCT8(DCT) / BruunFFT8(Bruun FFT) / MDCT7(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换(I型快速计算+FFT奇对称扩展+高效正弦变换)
 */
class DST8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForward = 0;
        int numInverse = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST8(QObject *parent = nullptr);
    ~DST8() override;

    /** @brief Configure transform size N */
    bool configure(int N);

    /** @brief Forward DST-I: x[n] -> X[k], k=1..N-1 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse DST-I: X[k] -> x[n] */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Forward DST-I in-place */
    void forwardInPlace(QVector<double>& data);

    /** @brief Inverse DST-I in-place */
    void inverseInPlace(QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief DST-I via FFT with odd-symmetry extension */
    void dstI(QVector<double>& data) const;

    /** @brief Radix-2 FFT butterfly */
    void fftButterfly(QVector<double>& re, QVector<double>& im) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;

    /** @brief Check if value is power of 2 */
    static bool isPowerOf2(int n);
};
