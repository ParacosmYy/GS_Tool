/**
 * @file RaderFFT5.h
 * @brief Rader算法(Winograd短卷积加速素数长度DFT) — Rader's Algorithm with Winograd Short Convolution for Prime-Length DFT Acceleration
 *
 * 功能: 实现Rader算法，支持素数长度DFT加速、
 *       Winograd短卷积优化和循环卷积计算。
 *
 * 协作: MixedRadixFFT5(混合基FFT) / WinogradFFT5(Winograd FFT) / ChirpZ6(Chirp-Z变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader算法(Winograd短卷积加速素数DFT)
 */
class RaderFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int convolutionSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT5(QObject *parent = nullptr);
    ~RaderFFT5() override;

    void setTransformSize(int n);

    /** @brief Check if n is prime */
    static bool isPrime(int n);

    /** @brief Find primitive root modulo n */
    static int primitiveRoot(int n);

    /** @brief Generate permutation indices via primitive root */
    QVector<int> generatePermutation(int n) const;

    /** @brief Forward DFT for prime-length input */
    void forward(QVector<double>& re, QVector<double>& im);

    /** @brief Inverse DFT for prime-length input */
    void inverse(QVector<double>& re, QVector<double>& im);

    /** @brief Compute cyclic convolution via direct method */
    QVector<double> cyclicConvolve(const QVector<double>& a,
                                   const QVector<double>& b) const;

    /** @brief Winograd short convolution for length-N */
    QVector<double> winogradConvolve(const QVector<double>& a,
                                     const QVector<double>& b) const;

    /** @brief Precompute twiddle factors for given prime */
    void precompute(int prime);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int convSize, double timeMs);

private:
    int m_size = 7;
    int m_convSize = 0;
    int m_primRoot = 0;

    // Precomputed tables
    QVector<int> m_perm;         // g^k mod p permutation
    QVector<double> m_twRe;      // cos/sin twiddle for convolution
    QVector<double> m_twIm;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modular exponentiation */
    static int modPow(int base, int exp, int mod);
};
