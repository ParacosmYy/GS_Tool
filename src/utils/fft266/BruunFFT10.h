/**
 * @file BruunFFT10.h
 * @brief Bruun FFT(多项式残差分解mod x^N-1递归实值快速变换) — Bruun FFT with Polynomial Residue Factorization Modulo x^N-1 for Recursive Real-Valued Fast Transform
 *
 * 功能: 实现Bruun FFT(Bruun FFT)，采用多项式残差分解(polynomial residue factorization)
 *       模x^N-1(modulo x^N-1)实现递归实值快速变换(recursive real-valued fast transform)。
 *
 * 协作: PrimeFactorFFT10(素因子FFT) / SplitRadixFFT9(分裂基数FFT) / RaderFFT10(Rader FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun FFT(多项式残差分解mod x^N-1递归实值快速变换)
 */
class BruunFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT10(QObject *parent = nullptr);
    ~BruunFFT10() override;

    /** @brief Forward real-valued DFT using Bruun's algorithm */
    QVector<double> transform(const QVector<double>& input);

    /** @brief Inverse real-valued DFT */
    QVector<double> inverseTransform(const QVector<double>& spectrum);

    /** @brief Get spectrum as complex pairs (re, im interleaved) */
    QVector<double> transformComplex(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int stages, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive Bruun DFT on polynomial coefficients */
    void bruunRecursive(const QVector<double>& input, QVector<double>& output,
                         int n, int stride) const;

    /** @brief Recursive inverse Bruun transform */
    void bruunInverseRecursive(const QVector<double>& input, QVector<double>& output,
                                int n, int stride) const;

    /** @brief Compute twiddle factors for Bruun factorization */
    QVector<double> bruunTwiddles(int n) const;

    /** @brief Base case: DFT of size 2 */
    void baseDFT2(const double* in, double* out, int stride) const;

    /** @brief Base case: DFT of size 4 */
    void baseDFT4(const double* in, double* out, int stride) const;

    /** @brief Pad input to next power of 2 */
    static int nextPow2(int n);

    /** @brief Compute number of butterfly stages */
    static int numStages(int n);
};
