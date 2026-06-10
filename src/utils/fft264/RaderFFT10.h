/**
 * @file RaderFFT10.h
 * @brief Rader FFT(Bluestein chirp-z任意素数长度DFT卷积计算) — Rader FFT with Bluestein Chirp-z Extension for Arbitrary Prime-length DFT via Convolution-based Computation
 *
 * 功能: 实现Rader FFT(Rader FFT算法)，采用Bluestein chirp-z扩展(Bluestein chirp-z
 *       extension)通过卷积计算(convolution-based computation)实现任意素数长度DFT
 *       (arbitrary prime-length DFT)。
 *
 * 协作: SplitRadixFFT9(分裂基数FFT) / Goertzel10(Goertzel算法) / WinogradFFT8(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Rader FFT(Bluestein chirp-z任意素数长度DFT卷积计算)
 */
class RaderFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int paddedSize = 0;
        bool isPrime = false;
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

    explicit RaderFFT10(QObject *parent = nullptr);
    ~RaderFFT10() override;

    /** @brief Compute forward DFT of arbitrary length using Rader/Bluestein */
    QVector<Complex> transform(const QVector<double>& input);

    /** @brief Compute inverse DFT */
    QVector<double> inverseTransform(const QVector<Complex>& spectrum);

    /** @brief Check if n is prime */
    static bool isPrime(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformComputed(int size, int paddedSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute DFT directly (O(n^2)) for small sizes */
    QVector<Complex> directDFT(const QVector<Complex>& input) const;

    /** @brief Rader algorithm for prime-length DFT */
    QVector<Complex> raderDFT(const QVector<Complex>& input) const;

    /** @brief Bluestein chirp-z for arbitrary length */
    QVector<Complex> bluesteinDFT(const QVector<Complex>& input) const;

    /** @brief Find primitive root modulo p */
    int primitiveRoot(int p) const;

    /** @brief Power-mod: base^exp mod m */
    int powMod(int base, int exp, int m) const;

    /** @brief Cooley-Tukey radix-2 FFT (power of 2) */
    QVector<Complex> fftPower2(const QVector<Complex>& input) const;

    /** @brief Next power of 2 >= n */
    int nextPower2(int n) const;

    /** @brief Complex multiply by conjugate */
    Complex conj(const Complex& c) const { return {c.re, -c.im}; }
};
