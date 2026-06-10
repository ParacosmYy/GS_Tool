/**
 * @file PrimeFactorFFT10.h
 * @brief 素因子FFT(Winograd短DFT模块与互素分解索引映射N=pq变换) — Prime Factor FFT with Winograd Short-DFT Modules and Index Mapping via Coprime Decomposition for N=pq Transforms
 *
 * 功能: 实现素因子FFT(Prime factor FFT)，采用Winograd短DFT模块(Winograd short-DFT
 *       modules)和互素分解索引映射(index mapping via coprime decomposition)实现
 *       N=pq变换(N=pq transforms)。
 *
 * 协作: RaderFFT10(Rader FFT) / SplitRadixFFT9(分裂基数FFT) / Goertzel10(Goertzel算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT(Winograd短DFT模块与互素分解索引映射N=pq变换)
 */
class PrimeFactorFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int factorP = 0;
        int factorQ = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Complex number */
    struct Complex {
        double re = 0.0;
        double im = 0.0;
        Complex operator+(const Complex& o) const { return {re + o.re, im + o.im}; }
        Complex operator-(const Complex& o) const { return {re - o.re, im - o.im}; }
        Complex operator*(const Complex& o) const {
            return {re * o.re - im * o.im, re * o.im + im * o.re};
        }
    };

    explicit PrimeFactorFFT10(QObject *parent = nullptr);
    ~PrimeFactorFFT10() override;

    /** @brief Compute forward DFT using prime factor decomposition */
    QVector<Complex> transform(const QVector<double>& input);

    /** @brief Compute inverse DFT */
    QVector<double> inverseTransform(const QVector<Complex>& spectrum);

    /** @brief Find coprime factorization n = p * q, returns (p,q) or (0,0) */
    QPair<int, int> factorize(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int p, int q, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Modular inverse of a modulo m (extended Euclidean) */
    int modInverse(int a, int m) const;

    /** @brief Check if two numbers are coprime */
    static bool isCoprime(int a, int b);

    /** @brief Direct DFT for small sizes */
    QVector<Complex> directDFT(const QVector<Complex>& input) const;

    /** @brief Winograd short-DFT for size 2 */
    QVector<Complex> winogradDFT2(const QVector<Complex>& input) const;

    /** @brief Winograd short-DFT for size 3 */
    QVector<Complex> winogradDFT3(const QVector<Complex>& input) const;

    /** @brief Winograd short-DFT for size 5 */
    QVector<Complex> winogradDFT5(const QVector<Complex>& input) const;

    /** @brief Dispatch to appropriate short-DFT */
    QVector<Complex> shortDFT(const QVector<Complex>& input) const;

    /** @brief Complex conjugate */
    Complex conj(const Complex& c) const { return {c.re, -c.im}; }
};
