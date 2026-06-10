/**
 * @file PrimeFactorFFT10.cpp
 * @brief PrimeFactorFFT10 实现
 *
 * 实现素因子FFT：Winograd短DFT模块与互素分解索引映射N=pq变换。
 */

#include "utils/fft265/PrimeFactorFFT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT10::PrimeFactorFFT10(QObject *parent)
    : QObject(parent) {}

PrimeFactorFFT10::~PrimeFactorFFT10() = default;

/* ---- Modular arithmetic ---- */

int PrimeFactorFFT10::modInverse(int a, int m) const
{
    int oldR = a, r = m, oldS = 1, s = 0;
    while (r != 0) {
        int q = oldR / r;
        int temp = r; r = oldR - q * r; oldR = temp;
        temp = s; s = oldS - q * s; oldS = temp;
    }
    return ((oldS % m) + m) % m;
}

bool PrimeFactorFFT10::isCoprime(int a, int b)
{
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a == 1;
}

/* ---- Coprime factorization ---- */

QPair<int, int> PrimeFactorFFT10::factorize(int n) const
{
    // Find coprime p, q such that p * q = n
    for (int p = 2; p * p <= n; ++p) {
        if (n % p != 0) continue;
        int q = n / p;
        if (isCoprime(p, q)) return {p, q};
    }
    return {0, 0};  // No coprime decomposition found
}

/* ---- Direct DFT ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::directDFT(
    const QVector<Complex>& input) const
{
    int n = input.size();
    QVector<Complex> output(n);
    for (int k = 0; k < n; ++k) {
        Complex sum;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            Complex tw = {qCos(angle), qSin(angle)};
            sum = sum + input[i] * tw;
        }
        output[k] = sum;
    }
    return output;
}

/* ---- Winograd DFT-2 (2 mult, 4 add) ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::winogradDFT2(
    const QVector<Complex>& input) const
{
    QVector<Complex> out(2);
    out[0] = input[0] + input[1];
    out[1] = input[0] - input[1];
    return out;
}

/* ---- Winograd DFT-3 (3 mult) ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::winogradDFT3(
    const QVector<Complex>& input) const
{
    // W = e^{-2*pi*i/3}
    const double c60 = 0.5;          // cos(60)
    const double s60 = qSqrt(3.0) / 2.0;  // sin(60)

    Complex t1 = input[1] + input[2];
    Complex t2 = input[0] - Complex{t1.re * c60, t1.im * c60};
    Complex t3 = Complex{(input[1].re - input[2].re) * s60,
                          (input[1].im - input[2].im) * s60};

    QVector<Complex> out(3);
    out[0] = input[0] + t1;
    out[1] = t2 + Complex{-t3.im, t3.re};  // Multiply by j
    out[2] = t2 + Complex{t3.im, -t3.re};
    return out;
}

/* ---- Winograd DFT-5 (5 mult) ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::winogradDFT5(
    const QVector<Complex>& input) const
{
    // Use direct DFT with precomputed twiddles for size 5
    const double c72 = qCos(2.0 * M_PI / 5.0);
    const double s72 = qSin(2.0 * M_PI / 5.0);
    const double c144 = qCos(4.0 * M_PI / 5.0);
    const double s144 = qSin(4.0 * M_PI / 5.0);

    QVector<Complex> out(5);
    out[0] = input[0];
    for (int i = 1; i < 5; ++i) out[0] = out[0] + input[i];

    // Compute using Goertzel-like short transforms
    Complex t1 = input[1] + input[4];
    Complex t2 = input[2] + input[3];
    Complex t3 = input[1] - input[4];
    Complex t4 = input[2] - input[3];

    Complex t5 = t1 + t2;
    Complex t6 = Complex{(t1.re - t2.re) * c72 - t3.im * s72 + t4.im * s144,
                          (t1.im - t2.im) * c72 + t3.re * s72 - t4.re * s144};
    Complex t7 = Complex{(t1.re - t2.re) * c144 - t3.im * s144 + t4.im * s72,
                          (t1.im - t2.im) * c144 + t3.re * s144 - t4.re * s72};
    Complex t8 = Complex{t3.re * s72 + t4.re * s144,
                          t3.im * s72 + t4.im * s144};
    Complex t9 = Complex{t3.re * s144 + t4.re * s72,
                          t3.im * s144 + t4.im * s72};

    out[1] = input[0] + Complex{t5.re * c72, t5.im * c72} +
             Complex{-t8.im, t8.re};
    out[2] = input[0] + Complex{t5.re * c144, t5.im * c144} +
             Complex{-t9.im, t9.re};
    out[3] = input[0] + Complex{t5.re * c144, t5.im * c144} +
             Complex{t9.im, -t9.re};
    out[4] = input[0] + Complex{t5.re * c72, t5.im * c72} +
             Complex{t8.im, -t8.re};

    return out;
}

/* ---- Dispatch short-DFT ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::shortDFT(
    const QVector<Complex>& input) const
{
    int n = input.size();
    if (n == 2) return winogradDFT2(input);
    if (n == 3) return winogradDFT3(input);
    if (n == 5) return winogradDFT5(input);
    return directDFT(input);
}

/* ---- Forward transform ---- */

QVector<PrimeFactorFFT10::Complex> PrimeFactorFFT10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<Complex> cx(n);
    for (int i = 0; i < n; ++i) cx[i] = {input[i], 0.0};

    QVector<Complex> result;
    int p = 0, q = 0;

    auto [fp, fq] = factorize(n);
    if (fp > 1 && fq > 1) {
        p = fp; q = fq;
        // Ruritanian index mapping for PFA
        int pInv = modInverse(p, q);
        int qInv = modInverse(q, p);

        // Stage 1: p DFTs of length q (column transforms)
        QVector<QVector<Complex>> colData(p);
        for (int i = 0; i < p; ++i) {
            colData[i].resize(q);
            for (int j = 0; j < q; ++j) {
                // Input index mapping: n1 = i, n2 = j
                int idx = (i * q + j * p) % n;
                colData[i][j] = cx[idx];
            }
            colData[i] = shortDFT(colData[i]);
        }

        // Stage 2: q DFTs of length p (row transforms with twiddle-free)
        QVector<QVector<Complex>> rowData(q);
        for (int j = 0; j < q; ++j) {
            rowData[j].resize(p);
            for (int i = 0; i < p; ++i)
                rowData[j][i] = colData[i][j];
            rowData[j] = shortDFT(rowData[j]);
        }

        // Output index mapping
        result.resize(n);
        for (int k1 = 0; k1 < p; ++k1) {
            for (int k2 = 0; k2 < q; ++k2) {
                int idx = (k1 * q * qInv + k2 * p * pInv) % n;
                result[idx] = rowData[k2][k1];
            }
        }
    } else {
        result = directDFT(cx);
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.factorP = p;
    m_stats.factorQ = q;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, p, q, elapsed);
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> PrimeFactorFFT10::inverseTransform(const QVector<Complex>& spectrum)
{
    int n = spectrum.size();
    QVector<Complex> conjInput(n);
    for (int i = 0; i < n; ++i) conjInput[i] = conj(spectrum[i]);

    // Forward DFT on conjugated input
    QVector<double> realIn(n);
    for (int i = 0; i < n; ++i) realIn[i] = conjInput[i].re;

    // Use forward transform (we reconstitute complex from real trick)
    QVector<Complex> fwd(n);
    for (int i = 0; i < n; ++i) fwd[i] = conjInput[i];

    auto [fp, fq] = factorize(n);
    QVector<Complex> result;
    if (fp > 1 && fq > 1) {
        int pInv = modInverse(fp, fq);
        int qInv = modInverse(fq, fp);
        QVector<QVector<Complex>> colData(fp);
        for (int i = 0; i < fp; ++i) {
            colData[i].resize(fq);
            for (int j = 0; j < fq; ++j) {
                int idx = (i * fq + j * fp) % n;
                colData[i][j] = fwd[idx];
            }
            colData[i] = shortDFT(colData[i]);
        }
        QVector<QVector<Complex>> rowData(fq);
        for (int j = 0; j < fq; ++j) {
            rowData[j].resize(fp);
            for (int i = 0; i < fp; ++i)
                rowData[j][i] = colData[i][j];
            rowData[j] = shortDFT(rowData[j]);
        }
        result.resize(n);
        for (int k1 = 0; k1 < fp; ++k1)
            for (int k2 = 0; k2 < fq; ++k2) {
                int idx = (k1 * fq * qInv + k2 * fp * pInv) % n;
                result[idx] = rowData[k2][k1];
            }
    } else {
        result = directDFT(fwd);
    }

    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = conj(result[i]).re / n;
    return out;
}

/* ---- Reset ---- */

void PrimeFactorFFT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
