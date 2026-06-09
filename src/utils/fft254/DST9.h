/**
 * @file DST9.h
 * @brief 离散正弦变换(Type-III快速预/后旋转FFT分解+纯正弦输出优化) — DST with Type-III Fast Computation via Pre/Post-Twiddle FFT Decomposition and Sine-Only Output Optimization
 *
 * 功能: 实现离散正弦变换(DST)，支持Type-III快速计算(Type-III fast
 *       computation)通过预旋转/后旋转FFT分解(pre/post-twiddle FFT
 *       decomposition)降低计算复杂度，纯正弦输出优化(sine-only
 *       output optimization)消除余弦分量。
 *
 * 协作: DCT9(离散余弦变换) / BruunFFT9(Bruun FFT) / MDCT7(改进DCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 离散正弦变换(Type-III+预/后旋转FFT分解)
 */
class DST9 : public QObject {
    Q_OBJECT

public:
    /** @brief DST type selector */
    enum Type { TypeI = 1, TypeII = 2, TypeIII = 3 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numTypeIII = 0;
        int numTwiddleOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST9(QObject *parent = nullptr);
    ~DST9() override;

    /** @brief Prepare transform for size N */
    bool prepare(int n, Type type = TypeII);

    /** @brief Forward DST transform */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse DST transform */
    QVector<double> inverse(const QVector<double>& coefficients);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int type, double timeMs);

private:
    int m_size = 0;
    Type m_type = TypeII;
    QVector<double> m_sinTable;   // Precomputed sines
    QVector<double> m_twiddleRe;  // Pre-twiddle real part
    QVector<double> m_twiddleIm;  // Pre-twiddle imaginary part

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute sine table and twiddle factors */
    void precompute();

    /** @brief DST-I direct computation */
    QVector<double> dstI(const QVector<double>& x) const;

    /** @brief DST-II direct computation */
    QVector<double> dstII(const QVector<double>& x) const;

    /** @brief DST-III via pre/post-twiddle FFT decomposition */
    QVector<double> dstIII(const QVector<double>& x) const;

    /** @brief Simple FFT (Cooley-Tukey radix-2 DIT) */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Check if n is power of 2 */
    static bool isPowerOf2(int n);
};
