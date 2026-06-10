/**
 * @file PrimeFactorFFT11.h
 * @brief 素因子FFT算法(Good-Thomas映射与Winograd嵌套短卷积的无乘法DFT) — Prime Factor FFT with Good-Thomas Mapping and Winograd Nested Short Convolution for Multiplication-free DFT
 *
 * 功能: 实现素因子FFT算法(Prime factor FFT)，采用Good-Thomas映射(Good-Thomas mapping)
 *       与Winograd嵌套短卷积(Winograd nested short convolution)实现无乘法DFT(multiplication-free DFT)。
 *
 * 协作: RaderFFT11(Rader FFT) / SplitRadixFFT10(分裂基数FFT) / MixedRadixFFT10(混合基数FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT算法(Good-Thomas映射与Winograd嵌套短卷积)
 */
class PrimeFactorFFT11 : public QObject {
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
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT11(QObject *parent = nullptr);
    ~PrimeFactorFFT11() override;

    /** @brief Set transform length N (must decompose into coprime factors) */
    void setLength(int n);

    /** @brief Compute forward DFT using prime factor algorithm */
    FFTResult transform(const QVector<double>& input);

    /** @brief Compute inverse DFT */
    QVector<double> inverseTransform(const FFTResult& spectrum);

    /** @brief Factorize N into coprime factors */
    QVector<int> factorize(int n) const;

    /** @brief Get current factors */
    QVector<int> factors() const { return m_factors; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int n, int numFactors, double timeMs);

private:
    int m_N = 60;       // Default: 60 = 3 * 4 * 5
    QVector<int> m_factors;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed Good-Thomas index mappings */
    QVector<int> m_rowMap;
    QVector<int> m_colMap;

    /** @brief Extended GCD for index mapping */
    int extGCD(int a, int b, int& x, int& y) const;

    /** @brief Precompute Good-Thomas mappings */
    void precomputeMappings();

    /** @brief Winograd short DFT for prime/small lengths */
    void winogradShortDFT(QVector<double>& real, QVector<double>& imag, int len) const;

    /** @brief Compute W_N^k = exp(-j*2*pi*k/N) */
    void twiddle(double angle, double& cosVal, double& sinVal) const;
};
