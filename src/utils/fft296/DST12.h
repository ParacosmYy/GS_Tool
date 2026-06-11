/**
 * @file DST12.h
 * @brief 离散正弦变换(FFT反射与奇对称扩展实现II型离散正弦变换) — DST with Fast Sine Transform via FFT Reflection and Odd-symmetry Extension for Type-II Discrete Sine Transform Computation
 *
 * 功能: 实现离散正弦变换(DST)，采用FFT反射(FFT reflection)
 *       与奇对称扩展(odd-symmetry extension)实现II型DST(type-II DST)。
 *
 * 协作: DCT12(离散余弦变换) / BruunFFT12(Bruun FFT) / SplitRadixFFT11(分裂基FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class DST12 : public QObject {
    Q_OBJECT

public:
    /** @brief DST result */
    struct DSTResult {
        QVector<double> coefficients;
        double peakValue = 0.0;
        double energyRatio = 0.0;   // First coefficient energy / total energy
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST12(QObject *parent = nullptr);
    ~DST12() override;

    /** @brief Set transform size (rounded to power of 2) */
    void setSize(int N);

    /** @brief Forward DST-II transform */
    DSTResult forward(const QVector<double>& input);

    /** @brief Inverse DST-II (i.e., DST-III) */
    QVector<double> inverse(const QVector<double>& coefficients);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peak, double timeMs);

private:
    int m_N = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed sine twiddle factors */
    QVector<double> m_sinTwiddles;

    /** @brief FFT working buffers */
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
