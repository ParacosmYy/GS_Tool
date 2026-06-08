/**
 * @file PrimeFactorFFT8.h
 * @brief 素因子FFT(Winograd短N模块+互素复合长度嵌套求和) — Prime Factor FFT with Winograd Short-N Modules and Nested Summation for Coprime Composite Lengths
 *
 * 功能: 实现素因子FFT算法(Prime factor FFT)，采用Winograd短N模块(Winograd short-N modules)
 *       和嵌套求和(nested summation)处理互素复合长度(coprime composite lengths)的高效DFT变换，
 *       避免旋转因子乘法运算。
 *
 * 协作: RaderFFT8(Rader FFT) / SplitRadixFFT7(分裂基FFT) / Goertzel8(Goertzel算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT(Winograd短N模块+互素复合长度嵌套求和)
 */
class PrimeFactorFFT8 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex number */
    struct Complex {
        double re = 0.0;
        double im = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numForward = 0;
        int numInverse = 0;
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT8(QObject *parent = nullptr);
    ~PrimeFactorFFT8() override;

    /** @brief Configure for transform length N (must be product of pairwise coprime factors) */
    bool configure(int N);

    /** @brief Forward FFT */
    QVector<Complex> forward(const QVector<double>& input);

    /** @brief Forward FFT (complex input) */
    QVector<Complex> forwardComplex(const QVector<Complex>& input);

    /** @brief Inverse FFT */
    QVector<Complex> inverse(const QVector<Complex>& spectrum);

    /** @brief Get coprime factorization of N */
    QVector<int> factors() const;

    /** @brief Check if N can be decomposed into supported coprime factors */
    static bool isSupported(int N);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forwardCompleted(int N, double timeMs);
    void inverseCompleted(int N, double timeMs);

private:
    int m_N = 0;
    QVector<int> m_factors;       // coprime factors
    QVector<int> m_rurIndices;    // RUR index mapping

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factorize N into pairwise coprime factors from {2,3,4,5,7,8,9,16} */
    QVector<int> coprimeFactorize(int N) const;

    /** @brief Compute RUR (remainder/unique/reconstruction) index mapping */
    void computeRURMapping();

    /** @brief Winograd short-N DFT for N in {2,3,4,5,7,8,9,16} */
    QVector<Complex> winogradShortN(const QVector<Complex>& input, int n) const;

    /** @brief Winograd short-N inverse */
    QVector<Complex> winogradShortNInverse(const QVector<Complex>& input, int n) const;

    /** @brief Nested summation prime factor transform */
    QVector<Complex> pfaTransform(const QVector<Complex>& input, bool inverse) const;

    /** @brief Complex multiply */
    static Complex cmul(const Complex& a, const Complex& b);

    /** @brief Complex add */
    static Complex cadd(const Complex& a, const Complex& b);
};
