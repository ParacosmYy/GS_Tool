/**
 * @file DCT6.h
 * @brief 第二类离散余弦变换(半长度FFT快速计算+偶扩展输入重索引) — Type-II DCT with Fast Computation via Half-Length FFT and Even-Extended Input Reindexing
 *
 * 功能: 实现第二类离散余弦变换，支持半长度FFT快速计算、
 *       偶扩展输入重索引和正交归一化。
 *
 * 协作: PrimeFactorFFT6(素因子FFT) / MDCT3(MDCT) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 第二类离散余弦变换(半长度FFT快速计算+偶扩展输入重索引)
 */
class DCT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT6(QObject *parent = nullptr);
    ~DCT6() override;

    /** @brief Forward Type-II DCT (real input -> real output) */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse Type-II DCT (equivalent to Type-III DCT) */
    QVector<double> inverse(const QVector<double>& spectrum) const;

    /** @brief Orthonormal forward DCT (normalized by sqrt(2/N)) */
    QVector<double> forwardOrtho(const QVector<double>& input) const;

    /** @brief Orthonormal inverse DCT */
    QVector<double> inverseOrtho(const QVector<double>& spectrum) const;

    /** @brief Naive O(N^2) DCT-II for verification */
    static QVector<double> naiveDCTII(const QVector<double>& input);

    /** @brief Compute the next power-of-2 for FFT padding */
    static int nextPowerOfTwo(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int fftSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Internal FFT helper using Cooley-Tukey radix-2 */
    static void fftRadix2(QVector<QPair<double, double>>& data);

    /** @brief Even-extend and reindex input for half-length FFT */
    static QVector<QPair<double, double>> prepareEvenExtend(
        const QVector<double>& input);
};
