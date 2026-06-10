/**
 * @file RaderFFT11.h
 * @brief Rader FFT算法(Winograd短卷积与扩展原根的素数长度DFT循环相关变换) — Rader FFT with Winograd Short Convolution and Extended Primitive Root for Prime-length DFT via Cyclic Correlation
 *
 * 功能: 实现Rader FFT算法(Rader's FFT algorithm)，采用Winograd短卷积(Winograd short convolution)
 *       与扩展原根(extended primitive root)实现素数长度DFT循环相关变换(prime-length DFT via cyclic correlation)。
 *
 * 协作: SplitRadixFFT10(分裂基数FFT) / Goertzel11(Goertzel算法) / MixedRadixFFT10(混合基数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader FFT算法(Winograd短卷积与扩展原根)
 */
class RaderFFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        double magnitude(const int i) const;
        double phase(const int i) const;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int primeSize = 0;
        int primitiveRoot = 0;
        int convLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT11(QObject *parent = nullptr);
    ~RaderFFT11() override;

    /** @brief Set prime length N for transform */
    void setPrimeLength(int n);

    /** @brief Compute forward DFT using Rader's algorithm */
    FFTResult transform(const QVector<double>& input);

    /** @brief Compute inverse DFT */
    QVector<double> inverseTransform(const FFTResult& spectrum);

    /** @brief Find primitive root modulo p */
    int findPrimitiveRoot(int p) const;

    /** @brief Get current primitive root */
    int primitiveRoot() const { return m_primRoot; }

    /** @brief Get permutation indices from primitive root */
    QVector<int> generatePermutation(int p, int g) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, int primitiveRoot, double timeMs);

private:
    int m_primeN = 7;
    int m_primRoot = 3;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Precomputed tables
    QVector<int> m_permFwd;     // Forward permutation (exponent table)
    QVector<int> m_permInv;     // Inverse permutation
    QVector<double> m_twiddleReal; // Twiddle factors for convolution
    QVector<double> m_twiddleImag;

    /** @brief Check if n is prime */
    bool isPrime(int n) const;

    /** @brief Compute modular exponentiation */
    qint64 modPow(qint64 base, qint64 exp, qint64 mod) const;

    /** @brief Precompute tables for current prime */
    void precomputeTables();

    /** @brief Cyclic convolution via direct computation */
    void cyclicConvolve(const QVector<double>& aReal, const QVector<double>& aImag,
                         const QVector<double>& bReal, const QVector<double>& bImag,
                         QVector<double>& outReal, QVector<double>& outImag) const;

    /** @brief Winograd short convolution for small sizes */
    void winogradConvolve(const QVector<double>& aReal, const QVector<double>& aImag,
                            const QVector<double>& bReal, const QVector<double>& bImag,
                            QVector<double>& outReal, QVector<double>& outImag) const;
};
