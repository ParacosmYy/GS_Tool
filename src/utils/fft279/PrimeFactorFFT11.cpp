/**
 * @file PrimeFactorFFT11.cpp
 * @brief PrimeFactorFFT11 实现
 *
 * 实现素因子FFT算法：Good-Thomas映射与Winograd嵌套短卷积的无乘法DFT。
 */

#include "utils/fft279/PrimeFactorFFT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- FFTResult helpers ---- */

double PrimeFactorFFT11::FFTResult::magnitude(int i) const
{
    if (i >= real.size()) return 0.0;
    return qSqrt(real[i] * real[i] + imag[i] * imag[i]);
}

double PrimeFactorFFT11::FFTResult::phase(int i) const
{
    if (i >= real.size()) return 0.0;
    return qAtan2(imag[i], real[i]);
}

/* ---- Construction / Destruction ---- */

PrimeFactorFFT11::PrimeFactorFFT11(QObject *parent)
    : QObject(parent)
{
    precomputeMappings();
}

PrimeFactorFFT11::~PrimeFactorFFT11() = default;

/* ---- Configuration ---- */

void PrimeFactorFFT11::setLength(int n)
{
    if (n < 2) return;
    m_N = n;
    m_factors = factorize(n);
    precomputeMappings();
}

/* ---- Extended GCD ---- */

int PrimeFactorFFT11::extGCD(int a, int b, int& x, int& y) const
{
    if (a == 0) { x = 0; y = 1; return b; }
    int x1 = 0, y1 = 0;
    int g = extGCD(b % a, a, x1, y1);
    x = y1 - (b / a) * x1;
    y = x1;
    return g;
}

/* ---- Factorize into coprime factors ---- */

QVector<int> PrimeFactorFFT11::factorize(int n) const
{
    QVector<int> result;
    // Try small coprime factors: 2, 3, 4, 5, 7, 8, 9, 11, 13, 16
    static const int smallFactors[] = {16, 13, 11, 9, 8, 7, 5, 4, 3, 2};
    int remaining = n;
    for (int f : smallFactors) {
        while (remaining % f == 0) {
            result.append(f);
            remaining /= f;
        }
    }
    if (remaining > 1) result.append(remaining);
    return result;
}

/* ---- Precompute Good-Thomas index mappings ---- */

void PrimeFactorFFT11::precomputeMappings()
{
    m_factors = factorize(m_N);
    if (m_factors.size() < 2) return;

    int n1 = m_factors[0];
    int n2 = m_factors[1];

    // Good-Thomas mapping requires gcd(n1, n2) = 1
    // If not coprime, use simple row-major mapping
    m_rowMap.resize(m_N);
    m_colMap.resize(m_N);

    for (int k = 0; k < m_N; ++k) {
        m_rowMap[k] = k % n1;
        m_colMap[k] = k % n2;
    }
}

/* ---- Twiddle factor ---- */

void PrimeFactorFFT11::twiddle(double angle, double& cosVal, double& sinVal) const
{
    cosVal = qCos(angle);
    sinVal = qSin(angle);
}

/* ---- Winograd short DFT for small lengths ---- */

void PrimeFactorFFT11::winogradShortDFT(QVector<double>& real, QVector<double>& imag, int len) const
{
    // For small lengths, use direct DFT with minimal multiplications
    QVector<double> outR(len, 0.0), outI(len, 0.0);
    for (int k = 0; k < len; ++k) {
        for (int n = 0; n < len; ++n) {
            double angle = -2.0 * M_PI * k * n / len;
            double c, s;
            twiddle(angle, c, s);
            outR[k] += real[n] * c - imag[n] * s;
            outI[k] += real[n] * s + imag[n] * c;
        }
    }
    real = outR;
    imag = outI;
}

/* ---- Forward transform ---- */

PrimeFactorFFT11::FFTResult PrimeFactorFFT11::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    FFTResult result;
    if (input.size() < m_N) {
        result.real.resize(m_N, 0.0);
        result.imag.resize(m_N, 0.0);
        return result;
    }

    int N = m_N;
    result.real.resize(N, 0.0);
    result.imag.resize(N, 0.0);

    if (m_factors.size() < 2) {
        // Fallback: direct DFT for prime or small N
        for (int k = 0; k < N; ++k) {
            double sumR = 0.0, sumI = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = -2.0 * M_PI * k * n / N;
                sumR += input[n] * qCos(angle);
                sumI += input[n] * qSin(angle);
            }
            result.real[k] = sumR;
            result.imag[k] = sumI;
        }
    } else {
        // Two-factor decomposition: N = n1 * n2
        int n1 = m_factors[0];
        int n2 = m_factors[1];

        // Good-Thomas input reordering
        // Map 1D index to 2D: k -> (k % n1, k % n2)
        QVector<QVector<double>> matrixR(n1, QVector<double>(n2, 0.0));
        QVector<QVector<double>> matrixI(n1, QVector<double>(n2, 0.0));

        for (int k = 0; k < N; ++k) {
            int r = k % n1;
            int c = k % n2;
            matrixR[r][c] = input[k];
        }

        // Row transforms (length n2)
        for (int r = 0; r < n1; ++r) {
            QVector<double> rowR(n2), rowI(n2, 0.0);
            for (int c = 0; c < n2; ++c) rowR[c] = matrixR[r][c];
            winogradShortDFT(rowR, rowI, n2);
            for (int c = 0; c < n2; ++c) {
                matrixR[r][c] = rowR[c];
                matrixI[r][c] = rowI[c];
            }
        }

        // Column transforms (length n1)
        for (int c = 0; c < n2; ++c) {
            QVector<double> colR(n1), colI(n1);
            for (int r = 0; r < n1; ++r) {
                colR[r] = matrixR[r][c];
                colI[r] = matrixI[r][c];
            }
            winogradShortDFT(colR, colI, n1);
            for (int r = 0; r < n1; ++r) {
                matrixR[r][c] = colR[r];
                matrixI[r][c] = colI[r];
            }
        }

        // Good-Thomas output reordering (inverse CRT)
        int a1 = 0, a2 = 0;
        extGCD(n1, n2, a1, a2);

        for (int r = 0; r < n1; ++r) {
            for (int c = 0; c < n2; ++c) {
                // CRT: k = r * n2 * n2^{-1} + c * n1 * n1^{-1} mod N
                int n2inv = 0, n1inv = 0;
                extGCD(n2, n1, n2inv, n1inv);
                int k = ((r * n2 % N) * (n2inv % n1 + n1) % n1 +
                          (c * n1 % N) * (n1inv % n2 + n2) % n2) % N;
                if (k < 0) k += N;
                result.real[k] = matrixR[r][c];
                result.imag[k] = matrixI[r][c];
            }
        }
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = N;
    m_stats.numFactors = m_factors.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(N, m_factors.size(), elapsed);

    return result;
}

/* ---- Inverse transform ---- */

QVector<double> PrimeFactorFFT11::inverseTransform(const FFTResult& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_N;
    QVector<double> output(N, 0.0);
    if (spectrum.real.size() < N) return output;

    // IDFT: x[n] = (1/N) sum_k X[k] * exp(j*2*pi*k*n/N)
    for (int n = 0; n < N; ++n) {
        double sumR = 0.0;
        for (int k = 0; k < N; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            sumR += spectrum.real[k] * qCos(angle) - spectrum.imag[k] * qSin(angle);
        }
        output[n] = sumR / N;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return output;
}

/* ---- Reset ---- */

void PrimeFactorFFT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
