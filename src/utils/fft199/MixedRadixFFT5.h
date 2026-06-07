/**
 * @file MixedRadixFFT5.h
 * @brief 混合基FFT(自动因子分解策略+SIMD友好内存布局) — Mixed-Radix FFT with Automatic Factorization Strategy and SIMD-Friendly Memory Layout
 *
 * 功能: 实现混合基FFT，支持自动因子分解策略、
 *       SIMD友好内存布局和多基数Cooley-Tukey蝶形运算。
 *
 * 协作: WinogradFFT5(Winograd FFT) / HexFFT5(六角FFT) / ChirpZ6(Chirp-Z变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混合基FFT(自动因子分解+SIMD布局)
 */
class MixedRadixFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A radix factor in the decomposition */
    struct Factor {
        int radix = 0;
        int count = 0;
    };

    explicit MixedRadixFFT5(QObject *parent = nullptr);
    ~MixedRadixFFT5() override;

    void setTransformSize(int n);
    void setAllowInPlace(bool enable);

    /** @brief Factorize N into small radices */
    QVector<Factor> factorize(int n) const;

    /** @brief Forward FFT (complex-to-complex) */
    void forward(QVector<double>& re, QVector<double>& im);

    /** @brief Inverse FFT */
    void inverse(QVector<double>& re, QVector<double>& im);

    /** @brief Real-valued forward FFT, returns interleaved [re0,im0,re1,im1,...] */
    QVector<double> forwardReal(const QVector<double>& input);

    /** @brief Compute twiddle factors for given N */
    QVector<double> computeTwiddles(int n) const;

    /** @brief Compute bit-reversed permutation index */
    QVector<int> bitReversePermutation(int n, const QVector<Factor>& factors) const;

    /** @brief Reorder data using digit-reversal for mixed radix */
    void digitReverse(QVector<double>& re, QVector<double>& im,
                      const QVector<Factor>& factors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int factors, double timeMs);

private:
    int m_size = 1024;
    bool m_inPlace = true;

    // Precomputed tables
    QVector<double> m_twiddleRe;
    QVector<double> m_twiddleIm;
    QVector<int> m_permutation;
    QVector<Factor> m_factors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build twiddle and permutation tables */
    void buildTables();

    /** @brief DFT kernel for small radix (2,3,4,5,7) */
    void dftKernel(double* re, double* im, int radix, double twRe, double twIm) const;

    /** @brief Apply all butterfly stages */
    void butterfly(QVector<double>& re, QVector<double>& im) const;
};
