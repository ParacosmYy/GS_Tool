/**
 * @file MixedRadixFFT9.h
 * @brief 混合基数FFT(运行时因式分解旋转因子递归通用复合N变换) — Mixed-radix FFT with Runtime Factorization and Twiddle Factor Recursion for General Composite N Transforms
 *
 * 功能: 实现混合基数FFT(Mixed-radix FFT)，采用运行时因式分解
 *       (runtime factorization)和旋转因子递归(twiddle factor
 *       recursion)支持任意复合长度N的DFT变换。
 *
 * 协作: FFT4(FFT) / HexFFT9(六角FFT) / SlidingDFT9(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基数FFT(运行时因式分解旋转因子递归通用复合N变换)
 */
class MixedRadixFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        int numInverseTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT9(QObject *parent = nullptr);
    ~MixedRadixFFT9() override;

    /** @brief Forward transform (complex interleaved: [re0,im0,re1,im1,...]) */
    QVector<double> forward(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Inverse transform */
    QVector<double> inverse(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Get factors of current N */
    QVector<int> factors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, bool inverse, double timeMs);

private:
    int m_n = 0;
    QVector<int> m_factors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into prime/small factors */
    QVector<int> factorize(int n) const;

    /** @brief Recursive mixed-radix butterfly */
    void mixedRadixDFT(QVector<double>& real, QVector<double>& imag,
                        int n, int stride, bool inverse) const;

    /** @brief Compute single twiddle factor */
    void twiddle(int k, int n, double& re, double& im, bool inverse) const;

    /** @brief Permute output by digit-reversal for factors */
    void digitReverse(QVector<double>& real, QVector<double>& imag,
                       int n, const QVector<int>& factors) const;
};
