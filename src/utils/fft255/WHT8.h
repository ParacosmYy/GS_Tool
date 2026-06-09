/**
 * @file WHT8.h
 * @brief 沃尔什-哈达玛变换(自然序快速计算+序列号到自然序位逆序置换) — Walsh-Hadamard Transform with Natural-Ordered Fast Computation and Sequency-to-Natural Permutation via Bit-Reversal
 *
 * 功能: 实现沃尔什-哈达玛变换(Walsh-Hadamard Transform)，支持自然序快速
 *       计算(natural-ordered fast computation)和序列序(sequency order)输出，
 *       通过位逆序置换(bit-reversal permutation)完成序列号到自然序映射。
 *
 * 协作: DST9(离散正弦变换) / DCT9(离散余弦变换) / FFT3(快速傅里叶变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换(自然序快速+位逆序置换)
 */
class WHT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Output ordering mode */
    enum Order { Natural = 0, Sequency = 1, Hadamard = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numPermutations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT8(QObject *parent = nullptr);
    ~WHT8() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n, Order order = Natural);

    /** @brief Forward WHT transform */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse WHT transform (same as forward, scaled) */
    QVector<double> inverse(const QVector<double>& coefficients);

    /** @brief Get sequency permutation table */
    QVector<int> sequencyPermutation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int order, double timeMs);

private:
    int m_size = 0;
    Order m_order = Natural;
    QVector<int> m_sequencyPerm;  // Sequency-to-natural mapping

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place fast WHT (butterfly) */
    void fastWHT(QVector<double>& data) const;

    /** @brief Generate sequency permutation via Gray code bit-reversal */
    void buildSequencyPermutation();

    /** @brief Check if n is power of 2 */
    static bool isPowerOf2(int n);

    /** @brief Compute bit-reversal of an integer */
    int bitReverse(int x, int bits) const;
};
