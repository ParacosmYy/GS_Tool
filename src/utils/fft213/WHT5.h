/**
 * @file WHT5.h
 * @brief 沃尔什-哈达玛变换(序列序蝶形+原位Gray码置换) — Walsh-Hadamard Transform with Sequency-Ordered Butterfly and Fast In-Place Gray Code Permutation
 *
 * 功能: 实现快速沃尔什-哈达玛变换，支持序列序排列、
 *       原位Gray码置换和蝶形计算。
 *
 * 协作: DST6(离散正弦变换) / DCT6(离散余弦变换) / PrimeFactorFFT6(素因子FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换(序列序蝶形+原位Gray码置换)
 */
class WHT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int order = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT5(QObject *parent = nullptr);
    ~WHT5() override;

    /** @brief Natural-ordered WHT (Hadamard order) */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Sequency-ordered WHT (Walsh order) */
    QVector<double> forwardSequency(const QVector<double>& input) const;

    /** @brief Inverse WHT (same as forward, scaled by 1/N) */
    QVector<double> inverse(const QVector<double>& spectrum) const;

    /** @brief Inverse sequency-ordered WHT */
    QVector<double> inverseSequency(const QVector<double>& spectrum) const;

    /** @brief Naive O(N^2) WHT for verification */
    static QVector<double> naiveWHT(const QVector<double>& input);

    /** @brief In-place Gray code bit-reversal permutation */
    static void grayCodePermute(QVector<double>& data);

    /** @brief Compute Gray code of an integer */
    static quint32 grayCode(quint32 n);

    /** @brief Check if size is a power of 2 */
    static bool isPowerOfTwo(int n);

    /** @brief Next power of 2 >= n */
    static int nextPowerOfTwo(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int order, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place fast WHT butterfly computation */
    static void butterflyWHT(QVector<double>& data);
};
