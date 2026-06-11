/**
 * @file RaderFFT12.h
 * @brief Rader FFT算法(Winograd短DFT内核与置换旋转因子预计算实现素数长度快速卷积) — Rader FFT with Winograd Short-DFT Kernel and Permuted Twiddle Factor Precomputation for Prime-length Fast Convolution
 *
 * 功能: 实现Rader FFT算法(Rader FFT algorithm)，采用Winograd短DFT内核(Winograd short-DFT kernel)
 *       与置换旋转因子预计算(permuted twiddle factor precomputation)实现素数长度快速卷积(prime-length fast convolution)。
 *
 * 协作: Goertzel12(Goertzel算法) / SplitRadixFFT11(分裂基FFT) / BluesteinFFT11(Bluestein FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class RaderFFT12 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        double magnitude = 0.0;         // Peak magnitude
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT12(QObject *parent = nullptr);
    ~RaderFFT12() override;

    /** @brief Set transform size (must be prime for Rader optimization) */
    void setSize(int N);

    /** @brief Forward FFT on real input */
    FFTResult forward(const QVector<double>& input);

    /** @brief Forward FFT on complex input */
    FFTResult forwardComplex(const QVector<double>& realIn,
                              const QVector<double>& imagIn);

    /** @brief Inverse FFT */
    FFTResult inverse(const QVector<double>& realIn,
                       const QVector<double>& imagIn);

    /** @brief Fast convolution of two signals using Rader FFT */
    QVector<double> convolve(const QVector<double>& a, const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peakMag, double timeMs);

private:
    int m_N = 7;                       // Transform size (prime)
    Stats m_stats;
    double m_timeSum = 0.0;

    // Precomputed Rader permutation and Winograd data
    QVector<int> m_permA;              // Generator-based permutation a
    QVector<int> m_permB;              // Inverse permutation b
    QVector<double> m_twiddleReal;     // Precomputed twiddle factors (real)
    QVector<double> m_twiddleImag;     // Precomputed twiddle factors (imag)
    QVector<double> m_winogradReal;    // Winograd short-DFT kernel
    QVector<double> m_winogradImag;

    /** @brief Check if number is prime */
    bool isPrime(int n) const;

    /** @brief Find primitive root modulo n */
    int primitiveRoot(int n) const;

    /** @brief Precompute permutation and twiddle factors */
    void precompute();

    /** @brief Compute DFT for a single element (Winograd inner kernel) */
    void winogradShortDFT(const QVector<double>& inReal,
                           const QVector<double>& inImag,
                           QVector<double>& outReal,
                           QVector<double>& outImag) const;

    /** @brief Multiply complex arrays element-wise */
    void complexMultiply(const QVector<double>& ar, const QVector<double>& ai,
                          const QVector<double>& br, const QVector<double>& bi,
                          QVector<double>& cr, QVector<double>& ci) const;
};
