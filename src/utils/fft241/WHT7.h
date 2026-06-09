/**
 * @file WHT7.h
 * @brief 沃尔什-哈达玛变换(序率排序快速计算+格雷码比特逆序重索引) — Walsh-Hadamard Transform with Sequency-Ordered Fast Computation via Gray Code Bit-Reversal Reindexing
 *
 * 功能: 实现沃尔什-哈达玛变换(Walsh-Hadamard transform)，采用序率排序快速计算
 *       (sequency-ordered fast computation)通过Gray码比特逆序重索引(Gray code bit-reversal
 *       reindexing)将自然序Hadamard矩阵转换为序率序(Walsh序)，实现高效正交变换。
 *
 * 协作: DST8(离散正弦) / DCT8(离散余弦) / BruunFFT8(Bruun FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换(序率排序快速计算+格雷码比特逆序重索引)
 */
class WHT7 : public QObject {
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

    explicit WHT7(QObject *parent = nullptr);
    ~WHT7() override;

    /** @brief Configure transform size N (must be power of 2) */
    bool configure(int N);

    /** @brief Forward WHT (sequency-ordered) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse WHT (sequency-ordered) */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Forward WHT in-place */
    void forwardInPlace(QVector<double>& data);

    /** @brief Inverse WHT in-place */
    void inverseInPlace(QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;
    int m_log2N = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Natural-order fast WHT butterfly */
    void fastWHT(QVector<double>& data) const;

    /** @brief Compute Gray code of index */
    static int grayCode(int i);

    /** @brief Bit-reversal permutation */
    static int bitReverse(int val, int log2n);

    /** @brief Build sequency reindex table via Gray code */
    QVector<int> buildSequencyIndex() const;

    /** @brief Check if power of 2 */
    static bool isPowerOf2(int n);
};
