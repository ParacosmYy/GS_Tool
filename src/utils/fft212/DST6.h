/**
 * @file DST6.h
 * @brief 第二类离散正弦变换(奇对称扩展+快速正弦DCT计算流水线) — Type-II DST with Odd-Symmetric Extension and Fast Sine DCT-Based Computation Pipeline
 *
 * 功能: 实现第二类离散正弦变换，支持奇对称输入扩展、
 *       基于DCT的快速计算流水线和正交归一化。
 *
 * 协作: DCT6(离散余弦变换) / PrimeFactorFFT6(素因子FFT) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 第二类离散正弦变换(奇对称扩展+快速正弦DCT计算流水线)
 */
class DST6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST6(QObject *parent = nullptr);
    ~DST6() override;

    /** @brief Forward Type-II DST */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse Type-II DST (equivalent to Type-III DST) */
    QVector<double> inverse(const QVector<double>& spectrum) const;

    /** @brief Orthonormal forward DST */
    QVector<double> forwardOrtho(const QVector<double>& input) const;

    /** @brief Orthonormal inverse DST */
    QVector<double> inverseOrtho(const QVector<double>& spectrum) const;

    /** @brief Naive O(N^2) DST-II for verification */
    static QVector<double> naiveDSTII(const QVector<double>& input);

    /** @brief Next power of 2 */
    static int nextPowerOfTwo(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int fftSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Radix-2 FFT helper */
    static void fftRadix2(QVector<QPair<double, double>>& data);

    /** @brief Build odd-symmetric extended sequence for DCT-based DST */
    static QVector<double> buildOddSymmetric(const QVector<double>& input);
};
