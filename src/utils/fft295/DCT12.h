/**
 * @file DCT12.h
 * @brief 离散余弦变换(FFT约化快速余弦变换与偶扩展对称性实现II型DCT) — DCT with Fast Cosine Transform via FFT Reduction and Even-extension Symmetry for Type-II Discrete Cosine Transform
 *
 * 功能: 实现离散余弦变换(DCT)，采用FFT约化快速余弦变换(fast cosine transform via FFT reduction)
 *       与偶扩展对称性(even-extension symmetry)实现II型DCT(type-II DCT)。
 *
 * 协作: BruunFFT12(Bruun FFT) / SplitRadixFFT11(分裂基FFT) / RealFFT10(实数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class DCT12 : public QObject {
    Q_OBJECT

public:
    /** @brief DCT result */
    struct DCTResult {
        QVector<double> coefficients;
        double energyRatio = 0.0;   // DC energy / total energy
        double peakValue = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT12(QObject *parent = nullptr);
    ~DCT12() override;

    /** @brief Set transform size (will be rounded to power of 2) */
    void setSize(int N);

    /** @brief Forward DCT-II transform */
    DCTResult forward(const QVector<double>& input);

    /** @brief Inverse DCT-II (i.e., DCT-III) */
    QVector<double> inverse(const QVector<double>& coefficients);

    /** @brief Get DC component only */
    double dcComponent(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peak, double timeMs);

private:
    int m_N = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed cosine twiddle factors */
    QVector<double> m_cosTwiddles;

    /** @brief Precomputed scaling factors */
    QVector<double> m_scaleFactors;

    /** @brief Internal FFT buffers for even-extension */
    QVector<double> m_fftReal;
    QVector<double> m_fftImag;

    /** @brief Precompute twiddle factors */
    void precompute();

    /** @brief In-place FFT (Cooley-Tukey radix-2 DIT) */
    void fftInPlace(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Bit-reversal permutation */
    void bitReversePermute(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Reverse bits */
    int reverseBits(int val, int bits) const;
};
