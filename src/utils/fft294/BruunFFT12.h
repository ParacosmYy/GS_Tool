/**
 * @file BruunFFT12.h
 * @brief Bruun FFT(多项式余因子分解与递归余弦调制实现2的幂次实值变换) — Bruun FFT with Polynomial Residue Factorization and Recursive Cosine Modulation for Power-of-two Real-valued Transforms
 *
 * 功能: 实现Bruun FFT算法(Bruun FFT)，采用多项式余因子分解(polynomial residue factorization)
 *       与递归余弦调制(recursive cosine modulation)实现2的幂次实值变换(power-of-two real-valued transforms)。
 *
 * 协作: PrimeFactorFFT12(素因子FFT) / SplitRadixFFT11(分裂基FFT) / RealFFT10(实数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class BruunFFT12 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        double peakMagnitude = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT12(QObject *parent = nullptr);
    ~BruunFFT12() override;

    /** @brief Set transform size (must be power of 2) */
    void setSize(int N);

    /** @brief Forward real-valued FFT using Bruun algorithm */
    FFTResult forward(const QVector<double>& input);

    /** @brief Inverse transform from complex spectrum to real signal */
    FFTResult inverse(const QVector<double>& realIn,
                        const QVector<double>& imagIn);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peakMag, double timeMs);

private:
    int m_N = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bruun's cosine modulation coefficients for each stage */
    struct StageCoeffs {
        double cosVal;
        double sinVal;
    };

    QVector<QVector<StageCoeffs>> m_twiddles;  // Per-stage twiddle factors

    /** @brief Precompute Bruun twiddle factors */
    void precompute();

    /** @brief Recursive Bruun butterfly on real data */
    void bruunReal(double* real, double* imag, int N, int stage) const;

    /** @brief Recursive Bruun butterfly inverse */
    void bruunRealInverse(double* real, double* imag, int N, int stage) const;

    /** @brief 2-point DFT base case */
    void butterfly2(double* r0, double* i0, double* r1, double* i1) const;
};
