/**
 * @file BruunFFT7.h
 * @brief Bruun FFT(DFT因式分解为实/虚多项式残差对+级联蝶形) — Bruun FFT with DFT Factorization into Real/Imaginary Polynomial Residue Pairs and Cascade Butterfly
 *
 * 功能: 实现Bruun FFT算法，将DFT分解为实部和虚部多项式残差对，
 *       级联蝶形结构实现高效2^n点变换。
 *
 * 协作: PrimeFactorFFT7(素因子FFT) / SplitRadixFFT6(分裂基) / RaderFFT7(Rader FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun FFT(多项式残差+级联蝶形)
 */
class BruunFFT7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numStages = 0;
        int numButterflies = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT7(QObject *parent = nullptr);
    ~BruunFFT7() override;

    /** @brief Prepare transform for size N (must be power of 2) */
    bool prepare(int n);

    /** @brief Forward FFT: complex interleaved [re0,im0,re1,im1,...] */
    QVector<double> forward(const QVector<double>& input) const;

    /** @brief Inverse FFT */
    QVector<double> inverse(const QVector<double>& input) const;

    /** @brief Get prepared size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_n = 0;
    int m_stages = 0;

    // Precomputed polynomial residue coefficients per stage
    QVector<QVector<double>> m_realResidue;
    QVector<QVector<double>> m_imagResidue;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute polynomial residue coefficients for a stage */
    void computeResidueCoeffs(int stage, int blockSize,
                                QVector<double>& realCoeff,
                                QVector<double>& imagCoeff) const;

    /** @brief Bruun butterfly operation */
    void bruunButterfly(double* reA, double* imA,
                          double* reB, double* imB,
                          double realCoeff, double imagCoeff) const;

    /** @brief Apply cascade butterfly for all stages */
    void applyCascadeButterfly(QVector<double>& re, QVector<double>& im) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;
};
