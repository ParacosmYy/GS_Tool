/**
 * @file RaderFFT6.h
 * @brief Rader FFT(Bluestein啁啾预计算+缓存优化素数长度DFT) — Rader FFT with Bluestein Chirp Precomputation and Cache-Optimized Prime-Length DFT
 *
 * 功能: 实现Rader FFT算法，支持Bluestein啁啾预计算、
 *       素数长度DFT的缓存优化和混合基数FFT。
 *
 * 协作: ChirpZ7(Chirp Z变换) / SplitRadixFFT5(分裂基FFT) / WinogradFFT4(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Rader FFT(Bluestein啁啾预计算+缓存优化素数长度DFT)
 */
class RaderFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        bool lastWasPrime = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT6(QObject *parent = nullptr);
    ~RaderFFT6() override;

    /** @brief Precompute chirp table for given prime size */
    void precomputeChirp(int primeN);

    /** @brief Forward FFT (any size) */
    QVector<QPair<double, double>> forward(const QVector<double>& input) const;

    /** @brief Forward FFT on complex input */
    QVector<QPair<double, double>> forwardComplex(
        const QVector<QPair<double, double>>& input) const;

    /** @brief Inverse FFT */
    QVector<QPair<double, double>> inverse(
        const QVector<QPair<double, double>>& spectrum) const;

    /** @brief Rader's prime-length DFT using convolution */
    QVector<QPair<double, double>> raderPrime(
        const QVector<QPair<double, double>>& input) const;

    /** @brief Check if n is prime */
    static bool isPrime(int n);

    /** @brief Find primitive root modulo p */
    static int primitiveRoot(int p);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool wasPrime, double timeMs);

private:
    int m_cachedPrime = 0;
    QVector<QPair<double, double>> m_chirpSeq;
    QVector<QPair<double, double>> m_chirpConjSeq;
    QVector<int> m_permForward;
    QVector<int> m_permInverse;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT */
    static void fftImpl(QVector<QPair<double, double>>& data, bool inverse);

    /** @brief Complex multiply */
    static QPair<double, double> cmul(const QPair<double, double>& a,
                                       const QPair<double, double>& b);

    /** @brief Bit reverse */
    static int bitReverse(int x, int bits);

    /** @brief Next power of 2 >= n */
    static int nextPow2(int n);
};
