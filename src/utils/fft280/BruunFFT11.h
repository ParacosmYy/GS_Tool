/**
 * @file BruunFFT11.h
 * @brief Bruun FFT(递归多项式求值与Z变换留数的N点实值快速变换) — Bruun FFT with Recursive Polynomial Evaluation and Z-transform Residue for N-point Real-valued Fast Transform
 *
 * 功能: 实现Bruun FFT算法(Bruun FFT)，采用递归多项式求值(recursive polynomial evaluation)
 *       与Z变换留数(Z-transform residue)实现N点实值快速变换(N-point real-valued fast transform)。
 *
 * 协作: PrimeFactorFFT11(素因子FFT) / SplitRadixFFT10(分裂基数FFT) / RaderFFT11(Rader FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun FFT(递归多项式求值与Z变换留数)
 */
class BruunFFT11 : public QObject {
    Q_OBJECT

public:
    /** @brief FFT result */
    struct FFTResult {
        QVector<double> real;
        QVector<double> imag;
        double magnitude(int i) const;
        double phase(int i) const;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT11(QObject *parent = nullptr);
    ~BruunFFT11() override;

    /** @brief Set transform length N (must be power of 2) */
    void setLength(int n);

    /** @brief Forward Bruun FFT on real-valued input */
    FFTResult transform(const QVector<double>& input);

    /** @brief Inverse transform */
    QVector<double> inverseTransform(const FFTResult& spectrum);

    /** @brief Get number of butterfly stages */
    int numStages() const { return m_stages; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, int stages, double timeMs);

private:
    int m_N = 64;
    int m_stages = 6;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed twiddle factors */
    QVector<double> m_cosTable;
    QVector<double> m_sinTable;

    /** @brief Precompute twiddle tables */
    void precomputeTwiddles();

    /** @brief Recursive polynomial evaluation step */
    void polyEval(QVector<double>& real, QVector<double>& imag,
                   int start, int stride, int len, int stage) const;

    /** @brief Z-transform residue computation */
    void zResidue(QVector<double>& real, QVector<double>& imag,
                   int n) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& data) const;

    /** @brief Reverse bits */
    int revBits(int x, int bits) const;

    /** @brief Count log2 */
    int log2Int(int n) const;
};
