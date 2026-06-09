/**
 * @file BruunFFT8.h
 * @brief Bruun FFT(多项式因式分解+递归cos/sin调制实值2幂变换) — Bruun FFT with Polynomial Factorization and Recursive Cos/Sin Modulation for Real-Valued Power-of-Two Transforms
 *
 * 功能: 实现Bruun FFT算法(Bruun's FFT)，采用多项式因式分解(polynomial factorization)
 *       和递归cos/sin调制(recursive cos/sin modulation)实现实值2幂长度变换(real-valued
 *       power-of-two transforms)，利用z^n-1的多项式递归分解减少乘法次数。
 *
 * 协作: PrimeFactorFFT8(素因子FFT) / SplitRadixFFT7(分裂基FFT) / BruunUnscrambler(位序反转)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun FFT(多项式因式分解+递归cos/sin调制实值2幂变换)
 */
class BruunFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForward = 0;
        int numInverse = 0;
        int recursionDepth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT8(QObject *parent = nullptr);
    ~BruunFFT8() override;

    /** @brief Configure transform size N (must be power of 2) */
    bool configure(int N);

    /** @brief Forward real-valued FFT, returns interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Forward complex FFT */
    void forwardComplex(QVector<double>& re, QVector<double>& im);

    /** @brief Inverse complex FFT */
    void inverseComplex(QVector<double>& re, QVector<double>& im);

    /** @brief Inverse real-valued FFT from interleaved spectrum */
    QVector<double> inverse(const QVector<double>& spectrum);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;
    int m_log2N = 0;
    QVector<double> m_twiddleCos;  // precomputed cos twiddle factors
    QVector<double> m_twiddleSin;  // precomputed sin twiddle factors
    QVector<int> m_bitRev;         // bit-reversal permutation

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors via polynomial roots */
    void computeTwiddles();

    /** @brief Compute bit-reversal permutation table */
    void computeBitReversal();

    /** @brief Recursive Bruun butterfly with polynomial factorization */
    void bruunButterfly(double* re, double* im, int stride, int m, int offset) const;

    /** @brief Recursive Bruun transform core */
    void bruunRecurse(double* re, double* im, int n, int stride) const;

    /** @brief Apply bit-reversal permutation in-place */
    void bitReversePermute(QVector<double>& re, QVector<double>& im) const;

    /** @brief Merge conjugate-symmetric spectrum for real FFT */
    QVector<double> packRealSpectrum(const QVector<double>& re,
                                     const QVector<double>& im) const;

    /** @brief Unpack real spectrum to full complex */
    void unpackRealSpectrum(const QVector<double>& packed,
                            QVector<double>& re, QVector<double>& im) const;
};
