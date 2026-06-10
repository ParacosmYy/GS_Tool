/**
 * @file WHT9.h
 * @brief 沃尔什-哈达玛变换(序列序快速计算与Paley序蝶形结构) — Walsh-Hadamard Transform with Sequency-Ordered Fast Computation via Paley Ordering and Butterfly Structure
 *
 * 功能: 实现沃尔什-哈达玛变换(WHT)，采用序列序(sequency-ordered)快速计算(fast computation)
 *       通过Paley序(Paley ordering)与蝶形结构(butterfly structure)实现。
 *
 * 协作: DST10(离散正弦变换) / DCT10(离散余弦变换) / SplitRadixFFT9(分裂基数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 沃尔什-哈达玛变换(序列序快速计算与Paley序蝶形结构)
 */
class WHT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Ordering mode for Walsh functions */
    enum Ordering {
        Natural = 0,    // Natural (Hadamard) order
        Sequency,       // Walsh (sequency) order
        Paley,          // Dyadic (Paley) order
        BitReversal     // Bit-reversed order
    };

    explicit WHT9(QObject *parent = nullptr);
    ~WHT9() override;

    /** @brief Set output ordering mode */
    void setOrdering(Ordering mode);

    /** @brief Fast WHT in-place (input length must be power of 2) */
    QVector<double> transform(const QVector<double>& input);

    /** @brief Inverse WHT (same as forward for WHT) */
    QVector<double> inverseTransform(const QVector<double>& spectrum);

    /** @brief Compute WHT via Paley-ordered butterfly directly */
    QVector<double> transformPaley(const QVector<double>& input);

    /** @brief Reorder natural-order result to sequency order */
    QVector<double> toSequencyOrder(const QVector<double>& natural) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int stages, double timeMs);

private:
    Ordering m_ordering = Sequency;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute number of butterfly stages */
    static int numStages(int n);

    /** @brief Check if n is power of 2 */
    static bool isPow2(int n);

    /** @brief Bit-reversal permutation */
    static void bitReversePerm(QVector<double>& data);

    /** @brief Gray code for sequency ordering */
    static int grayCode(int x);

    /** @brief Bit-reverse an integer */
    static int bitReverse(int x, int bits);

    /** @brief Compute Paley-ordered index from natural index */
    static int paleyIndex(int idx, int bits);
};
