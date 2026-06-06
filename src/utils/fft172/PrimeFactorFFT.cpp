/**
 * @file PrimeFactorFFT.cpp
 * @brief PrimeFactorFFT 实现
 *
 * 实现素因子FFT：互素因子分解、Ruritanian索引映射、二维DFT分解。
 */

#include "utils/fft172/PrimeFactorFFT.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT::PrimeFactorFFT(QObject *parent)
    : QObject(parent)
{
}

PrimeFactorFFT::~PrimeFactorFFT() = default;

/* ---- Helpers ---- */

int PrimeFactorFFT::gcd(int a, int b)
{
    while (b) { int t = b; b = a % b; a = t; }
    return a;
}

int PrimeFactorFFT::modInverse(int a, int m)
{
    int x0 = 1, x1 = 0;
    int b = m;
    while (b) {
        int q = a / b;
        int t = b; b = a - q * b; a = t;
        t = x1; x1 = x0 - q * x1; x0 = t;
    }
    return (x0 % m + m) % m;
}

/* ---- Factorization ---- */

bool PrimeFactorFFT::isValidLength(int n) const
{
    int n1, n2;
    return factorize(n, n1, n2);
}

bool PrimeFactorFFT::factorize(int n, int& n1, int& n2) const
{
    if (n <= 0) return false;
    /* Try common coprime factorizations */
    for (n1 = 2; n1 < n; ++n1) {
        if (n % n1 != 0) continue;
        n2 = n / n1;
        if (gcd(n1, n2) == 1) return true;
    }
    n1 = n; n2 = 1;
    return true; /* Trivial decomposition */
}

/* ---- Index mapping (Ruritanian) ---- */

void PrimeFactorFFT::indexMap(int n1, int n2,
                               QVector<int>& rowToLinear,
                               QVector<int>& colToLinear) const
{
    int n = n1 * n2;
    rowToLinear.resize(n);
    colToLinear.resize(n);

    int n1inv = modInverse(n1, n2);
    int n2inv = modInverse(n2, n1);

    for (int k = 0; k < n; ++k) {
        int k1 = k % n1;
        int k2 = k % n2;
        int n_idx = (k1 * n2 * n2inv + k2 * n1 * n1inv) % n;
        rowToLinear[k] = n_idx;
    }

    /* Column-major mapping for transposed access */
    for (int k = 0; k < n; ++k) {
        int k1 = k % n1;
        int k2 = k % n2;
        int n_idx = (k2 * n1 * n1inv + k1 * n2 * n2inv) % n;
        colToLinear[k] = n_idx;
    }
}

/* ---- Short DFT (direct computation) ---- */

void PrimeFactorFFT::shortDFT(QVector<double>& r, QVector<double>& i,
                                int n, int stride, bool inverse) const
{
    if (n <= 1) return;

    QVector<double> rout(n), iout(n);
    double sign = inverse ? 1.0 : -1.0;

    for (int k = 0; k < n; ++k) {
        double rr = 0.0, ii = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = sign * 2.0 * M_PI * k * j / n;
            double cr = qCos(angle);
            double ci = qSin(angle);
            rr += r[j * stride] * cr - i[j * stride] * ci;
            ii += r[j * stride] * ci + i[j * stride] * cr;
        }
        rout[k] = rr;
        iout[k] = ii;
    }

    for (int k = 0; k < n; ++k) {
        r[k * stride] = rout[k];
        i[k * stride] = iout[k];
    }

    if (inverse) {
        for (int k = 0; k < n; ++k) {
            r[k * stride] /= n;
            i[k * stride] /= n;
        }
    }
}

/* ---- Main transform ---- */

bool PrimeFactorFFT::transform(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    int n1, n2;
    if (!factorize(n, n1, n2)) return false;

    QElapsedTimer timer;
    timer.start();

    /* Step 1: Index remap (Ruritanian) */
    QVector<int> mapRow, mapCol;
    indexMap(n1, n2, mapRow, mapCol);

    QVector<double> rTmp(n), iTmp(n);
    for (int k = 0; k < n; ++k) {
        rTmp[k] = real[mapRow[k]];
        iTmp[k] = imag[mapRow[k]];
    }
    real = rTmp;
    imag = iTmp;

    /* Step 2: Row DFTs (length n2) — n1 transforms */
    QVector<double> rRow(n2), iRow(n2);
    for (int i1 = 0; i1 < n1; ++i1) {
        for (int j = 0; j < n2; ++j) {
            rRow[j] = real[i1 + j * n1];
            iRow[j] = imag[i1 + j * n1];
        }
        shortDFT(rRow, iRow, n2, 1, false);
        for (int j = 0; j < n2; ++j) {
            real[i1 + j * n1] = rRow[j];
            imag[i1 + j * n1] = iRow[j];
        }
    }

    /* Step 3: Column DFTs (length n1) — n2 transforms */
    QVector<double> rCol(n1), iCol(n1);
    for (int i2 = 0; i2 < n2; ++i2) {
        int base = i2 * n1;
        for (int j = 0; j < n1; ++j) {
            rCol[j] = real[base + j];
            iCol[j] = imag[base + j];
        }
        shortDFT(rCol, iCol, n1, 1, false);
        for (int j = 0; j < n1; ++j) {
            real[base + j] = rCol[j];
            imag[base + j] = iCol[j];
        }
    }

    /* Step 4: Inverse index mapping */
    rTmp = real; iTmp = imag;
    for (int k = 0; k < n; ++k) {
        int src = mapRow[k];
        real[k] = rTmp[k];
        imag[k] = iTmp[k];
    }

    m_stats.totalTransforms++;
    m_stats.lastN = n;
    m_stats.lastN1 = n1;
    m_stats.lastN2 = n2;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(n, n1, n2);
    return true;
}

/* ---- Inverse transform ---- */

bool PrimeFactorFFT::inverseTransform(QVector<double>& real, QVector<double>& imag)
{
    /* Conjugate, forward transform, conjugate, divide by N */
    int n = real.size();
    for (int i = 0; i < n; ++i)
        imag[i] = -imag[i];

    if (!transform(real, imag)) return false;

    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
    return true;
}

/* ---- Statistics ---- */

void PrimeFactorFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
