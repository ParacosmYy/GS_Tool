/**
 * @file PrimeFactorFFT12.h
 * @brief 素因子FFT(无旋转映射与自排序索引置换实现互素因子复合长度变换) — Prime Factor FFT with Rotator-free Mapping and Self-sorting Index Permutation for Coprime Factor Composite Length Transform
 *
 * 功能: 实现素因子FFT算法(Prime factor FFT)，采用无旋转映射(rotator-free mapping)
 *       与自排序索引置换(self-sorting index permutation)实现互素因子复合长度变换(coprime factor composite length transform)。
 *
 * 协作: RaderFFT12(Rader FFT) / SplitRadixFFT11(分裂基FFT) / BluesteinFFT11(Bluestein FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class PrimeFactorFFT12 : public QObject {
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

    explicit PrimeFactorFFT12(QObject *parent = nullptr);
    ~PrimeFactorFFT12() override;

    /** @brief Set transform size (decomposed into coprime factors) */
    void setSize(int N);

    /** @brief Forward FFT on real input */
    FFTResult forward(const QVector<double>& input);

    /** @brief Forward FFT on complex input */
    FFTResult forwardComplex(const QVector<double>& realIn,
                               const QVector<double>& imagIn);

    /** @brief Inverse FFT */
    FFTResult inverse(const QVector<double>& realIn,
                        const QVector<double>& imagIn);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peakMag, double timeMs);

private:
    int m_N = 60;                       // Composite transform size
    Stats m_stats;
    double m_timeSum = 0.0;

    // Coprime factorization
    QVector<int> m_factors;             // Coprime factors of N
    QVector<int> m_ni;                  // N_i = N / factor_i
    QVector<int> m_mi;                  // Modular inverse of N_i mod factor_i

    // Index mapping tables (rotator-free Ruritanian mapping)
    QVector<int> m_inputMap;            // Self-sorting input index mapping
    QVector<int> m_outputMap;           // Output index mapping

    /** @brief Decompose N into pairwise coprime factors */
    QVector<int> factorize(int N) const;

    /** @brief Compute modular inverse: a^(-1) mod m */
    int modInverse(int a, int m) const;

    /** @brief Compute GCD */
    int gcd(int a, int b) const;

    /** @brief Precompute index mappings */
    void precompute();

    /** @brief Short DFT of length n (direct computation) */
    void shortDFT(double* real, double* imag, int n, bool inverse) const;

    /** @brief Multiply complex arrays element-wise */
    void complexMul(const double& ar, const double& ai,
                     const double& br, const double& bi,
                     double& cr, double& ci) const;
};
