/**
 * @file RaderFFT8.h
 * @brief Rader FFT(Bluestein chirp-z预计算+素数长度DFT循环卷积映射) — Rader FFT with Bluestein Chirp-Z Precomputation and Prime-Length DFT via Cyclic Convolution Mapping
 *
 * 功能: 实现Rader FFT算法(Rader's FFT algorithm)，通过Bluestein chirp-z预计算
 *       (Bluestein chirp-z precomputation)将素数长度DFT(prime-length DFT)映射为
 *       循环卷积(cyclic convolution)，支持任意长度的高效频域变换。
 *
 * 协作: SplitRadixFFT7(分裂基FFT) / Goertzel8(Goertzel算法) / WindowFunction4(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader FFT(Bluestein chirp-z预计算+素数长度DFT循环卷积映射)
 */
class RaderFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result for one transform */
    struct Complex {
        double re = 0.0;
        double im = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForward = 0;
        int numInverse = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT8(QObject *parent = nullptr);
    ~RaderFFT8() override;

    /** @brief Initialize for transform size N */
    bool configure(int N);

    /** @brief Forward FFT (time -> frequency) */
    QVector<Complex> forward(const QVector<double>& input);

    /** @brief Forward FFT (complex input) */
    QVector<Complex> forwardComplex(const QVector<Complex>& input);

    /** @brief Inverse FFT (frequency -> time) */
    QVector<Complex> inverse(const QVector<Complex>& spectrum);

    /** @brief Check if N is prime */
    static bool isPrime(int n);

    /** @brief Find next power of 2 >= n */
    static int nextPow2(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;
    int m_paddedLen = 0;       // padded to power of 2 for convolution

    // Bluestein chirp factors: chirp[k] = exp(j*pi*k^2/N)
    QVector<Complex> m_chirp;
    QVector<Complex> m_chirpPad;   // zero-padded for FFT convolution
    QVector<Complex> m_chirpFft;   // precomputed FFT of padded chirp

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute chirp sequence and its FFT */
    void precomputeChirp();

    /** @brief Power-of-2 FFT (Cooley-Tukey radix-2 DIT) */
    void fftPow2(QVector<Complex>& x, bool inverse) const;

    /** @brief Complex multiply */
    static Complex cmul(const Complex& a, const Complex& b);

    /** @brief Complex add */
    static Complex cadd(const Complex& a, const Complex& b);

    /** @brief Complex conjugate */
    static Complex conj(const Complex& a);
};
