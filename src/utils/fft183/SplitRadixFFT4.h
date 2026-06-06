/**
 * @file SplitRadixFFT4.h
 * @brief 分裂基FFT(基2+基4组合最优算术复杂度) — Split-radix FFT Combining Radix-2 and Radix-4 for Optimal Arithmetic Complexity
 *
 * 功能: 实现分裂基FFT算法，结合基2和基4分解实现最优乘法复杂度O(N log₂N)，
 *       支持位反转置换和原地(in-place)计算。
 *
 * 协作: MixedRadixFFT4(混合基) / BruunFFT4(Bruun) / WinogradFFT4(Winograd)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT处理器(基2+基4最优组合)
 */
class SplitRadixFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numButterflies = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplitRadixFFT4(QObject *parent = nullptr);
    ~SplitRadixFFT4() override;

    /** @brief Forward transform (complex interleaved) */
    void transform(const QVector<double>& inRe, const QVector<double>& inIm,
                   QVector<double>& outRe, QVector<double>& outIm);

    /** @brief Inverse transform */
    void inverseTransform(const QVector<double>& inRe, const QVector<double>& inIm,
                          QVector<double>& outRe, QVector<double>& outIm);

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;

    /** @brief Compute bit-reversed index */
    int bitReverseIndex(int idx, int log2N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int butterflies);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive split-radix core */
    void splitRadixCore(QVector<double>& re, QVector<double>& im,
                        int N, int stride, int offset);

    /** @brief Compute log2 of power-of-2 integer */
    int log2Int(int n) const;
};
