/**
 * @file MixedRadixFFT10.h
 * @brief 混合基数FFT(预计算旋转因子表与自排序原位置换缓存友好复合N变换) — Mixed-radix FFT with Precomputed Twiddle Table and Self-sorting In-place Permutation for Cache-friendly Composite-N Transforms
 *
 * 功能: 实现混合基数FFT(Mixed-radix FFT)，采用预计算旋转因子表(precomputed twiddle table)
 *       与自排序原位置换(self-sorting in-place permutation)实现缓存友好复合N变换(cache-friendly composite-N transforms)。
 *
 * 协作: HexFFT10(六角FFT) / ZoomFFT7(缩放FFT) / WinogradFFT6(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基数FFT(预计算旋转因子表与自排序原位置换)
 */
class MixedRadixFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numTransforms = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MixedRadixFFT10(QObject *parent = nullptr);
    ~MixedRadixFFT10() override;

    /** @brief Set transform size N (auto-factors to mixed radices) */
    void setSize(int n);

    /** @brief Forward FFT: returns complex interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Inverse FFT: returns complex interleaved */
    QVector<double> inverse(const QVector<double>& real, const QVector<double>& imag);

    /** @brief Get factorization of current N */
    QVector<int> factors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int size, int numTransforms, double timeMs);

private:
    int m_n = 256;
    QVector<int> m_factors;         // Radix factorization of N
    QVector<double> m_twiddles;     // Precomputed twiddle factors (cos, sin pairs)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into mixed radices (2,3,4,5) */
    void factorize();

    /** @brief Build precomputed twiddle table */
    void buildTwiddleTable();

    /** @brief Compute digit-reversed index for given factorization */
    int digitReverse(int index) const;

    /** @brief In-place digit-reversal permutation */
    void digitReversePermute(QVector<double>& re, QVector<double>& im) const;

    /** @brief Perform mixed-radix butterfly on one stage */
    void butterflyStage(QVector<double>& re, QVector<double>& im,
                        int stageIdx, bool inverse);

    /** @brief Small-N DFT kernel for radix r */
    void dftKernel(double* reOut, double* imOut,
                   const double* reIn, const double* imIn,
                   int radix, double twStart, double twStep, bool inverse) const;
};
