/**
 * @file BruunFFT4.h
 * @brief Bruun FFT(多项式残基分解mod(z^N-1)+递归实值变换) — Bruun's FFT via Polynomial Residue Factorization mod (z^N-1) with Recursive Real-valued Transform
 *
 * 功能: 实现Bruun FFT算法，通过多项式z^N-1的实系数因子分解进行递归变换，
 *       专用于纯实值信号，减少复数运算开销。
 *
 * 协作: WinogradFFT4(Winograd FFT) / RaderFFT4(Rader FFT) / SplitRadixFFT4(分裂基)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Bruun FFT处理器(多项式残基+递归实值变换)
 */
class BruunFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT4(QObject *parent = nullptr);
    ~BruunFFT4() override;

    /** @brief Bruun FFT for real-valued input (N must be power of 2) */
    QPair<QVector<double>, QVector<double>> transform(
        const QVector<double>& input);

    /** @brief Inverse Bruun FFT */
    QPair<QVector<double>, QVector<double>> inverseTransform(
        const QVector<double>& re, const QVector<double>& im);

    /** @brief 检查是否为2的幂 */
    bool isPowerOfTwo(int n) const;

    /** @brief 计算递归深度(级数) */
    int numStages(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int stages);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive Bruun butterfly for real signals */
    void bruunRecursive(const QVector<double>& in,
                         QVector<double>& outRe, QVector<double>& outIm,
                         int N, int stride, int offset);

    /** @brief Compute twiddle factors for real-valued transform */
    void realTwiddles(int N, QVector<double>& cosTable, QVector<double>& sinTable);

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& data);

    /** @brief Reverse bits */
    int reverseBits(int val, int bits) const;

    /** @brief Log2 for power-of-2 */
    int log2Int(int n) const;
};
