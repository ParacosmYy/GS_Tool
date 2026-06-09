/**
 * @file DCT8.h
 * @brief 离散余弦变换(II型快速计算+FFT重索引+偶扩展) — DCT with Type-II Fast Computation via FFT Reindexing and Even-Extension for Efficient Cosine Transform
 *
 * 功能: 实现离散余弦变换(DCT)，采用II型快速计算(type-II fast computation)通过FFT重索引
 *       (FFT reindexing)将DCT转换为等效FFT计算，利用偶扩展(even-extension)对称性消除
 *       冗余计算，实现高效余弦变换。
 *
 * 协作: BruunFFT8(Bruun FFT) / PrimeFactorFFT8(素因子FFT) / MDCT7(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散余弦变换(II型快速计算+FFT重索引+偶扩展)
 */
class DCT8 : public QObject {
    Q_OBJECT

public:
    /** @brief DCT type selector */
    enum Type { TypeII = 0, TypeIII = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForward = 0;
        int numInverse = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT8(QObject *parent = nullptr);
    ~DCT8() override;

    /** @brief Configure transform size N */
    bool configure(int N);

    /** @brief Forward DCT-II: x[n] -> X[k] */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse DCT-III: X[k] -> x[n] */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Forward DCT-II in-place */
    void forwardInPlace(QVector<double>& data);

    /** @brief Inverse DCT-III in-place */
    void inverseInPlace(QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;
    QVector<double> m_cosTable;   // precomputed cos(pi*k*(2n+1)/(2N))
    QVector<double> m_scale;      // normalization factors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute cosine table and scaling factors */
    void computeTables();

    /** @brief Internal FFT-based DCT-II via even-extension reindexing */
    void dctII(QVector<double>& data) const;

    /** @brief Internal FFT-based DCT-III via inverse reindexing */
    void dctIII(QVector<double>& data) const;

    /** @brief Radix-2 FFT butterfly for power-of-2 lengths */
    void fftButterfly(QVector<double>& re, QVector<double>& im) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;
};
