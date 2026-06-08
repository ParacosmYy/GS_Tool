/**
 * @file RaderFFT7.cpp
 * @brief RaderFFT7 实现
 *
 * 实现Rader FFT：素数长度Winograd短卷积、无旋转因子内层DFT。
 */

#include "utils/fft222/RaderFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT7::RaderFFT7(QObject *parent) : QObject(parent) {}
RaderFFT7::~RaderFFT7() = default;

/* ---- Prime check ---- */

bool RaderFFT7::isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

/* ---- Modular exponentiation ---- */

int RaderFFT7::modPow(int base, int exp, int mod)
{
    int result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

/* ---- Primitive root ---- */

int RaderFFT7::primitiveRoot(int n)
{
    if (n < 2) return 1;
    // Factorize n-1
    QVector<int> primeFactors;
    int temp = n - 1;
    for (int p = 2; p * p <= temp; ++p) {
        if (temp % p == 0) {
            primeFactors.append(p);
            while (temp % p == 0) temp /= p;
        }
    }
    if (temp > 1) primeFactors.append(temp);

    // Find smallest g such that g^((n-1)/q) != 1 mod n for all q
    for (int g = 2; g < n; ++g) {
        bool ok = true;
        for (int q : primeFactors) {
            if (modPow(g, (n - 1) / q, n) == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return n - 1;
}

/* ---- Factorize ---- */

QVector<int> RaderFFT7::factorize(int n)
{
    QVector<int> factors;
    for (int p = 2; p * p <= n; ++p) {
        while (n % p == 0) { factors.append(p); n /= p; }
    }
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Build permutation ---- */

void RaderFFT7::buildPermutation(int n, int g)
{
    m_permA.resize(n - 1);
    m_permB.resize(n - 1);
    for (int k = 0; k < n - 1; ++k) {
        m_permA[k] = modPow(g, k, n);
        m_permB[k] = modPow(g, n - 1 - k, n);
    }
}

/* ---- Precompute Winograd convolution kernel ---- */

void RaderFFT7::precomputeKernel()
{
    int n = m_n;
    int m = n - 1;

    // Twiddle factors W_N^g^(-k)
    m_kernelRe.resize(m);
    m_kernelIm.resize(m);
    for (int k = 0; k < m; ++k) {
        int exp = m_permB[k];
        double angle = -2.0 * M_PI * exp / n;
        m_kernelRe[k] = qCos(angle);
        m_kernelIm[k] = qSin(angle);
    }
}

/* ---- Prepare ---- */

void RaderFFT7::prepare(int n)
{
    m_n = qMax(1, n);
    m_stats.transformSize = m_n;
    m_stats.isPrime = isPrime(m_n);
    m_factors = factorize(m_n);
    m_stats.winogradSize = m_n - 1;

    if (m_stats.isPrime && m_n >= 3) {
        int g = primitiveRoot(m_n);
        m_stats.primitiveRoot = g;
        buildPermutation(m_n, g);
        precomputeKernel();
    }
}

/* ---- Circular convolution (FFT-based) ---- */

void RaderFFT7::circularConvolve(
    const QVector<double>& aRe, const QVector<double>& aIm,
    const QVector<double>& bRe, const QVector<double>& bIm,
    QVector<double>& outRe, QVector<double>& outIm) const
{
    int m = aRe.size();
    outRe.resize(m);
    outIm.resize(m);

    // Simple O(m^2) circular convolution (Winograd optimization for small m)
    for (int j = 0; j < m; ++j) {
        double sr = 0.0, si = 0.0;
        for (int k = 0; k < m; ++k) {
            int idx = (j - k + m) % m;
            sr += aRe[k] * bRe[idx] - aIm[k] * bIm[idx];
            si += aRe[k] * bIm[idx] + aIm[k] * bRe[idx];
        }
        outRe[j] = sr;
        outIm[j] = si;
    }
}

/* ---- Small-N DFT ---- */

void RaderFFT7::smallDFT(double* re, double* im, int n, int stride,
                           bool inverse) const
{
    double sign = inverse ? 1.0 : -1.0;
    QVector<double> tmpRe(n), tmpIm(n);
    for (int k = 0; k < n; ++k) {
        double sr = 0.0, si = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = sign * 2.0 * M_PI * j * k / n;
            double c = qCos(angle), s = qSin(angle);
            sr += re[j * stride] * c - im[j * stride] * s;
            si += re[j * stride] * s + im[j * stride] * c;
        }
        tmpRe[k] = sr;
        tmpIm[k] = si;
    }
    double scale = inverse ? 1.0 / n : 1.0;
    for (int k = 0; k < n; ++k) {
        re[k * stride] = tmpRe[k] * scale;
        im[k * stride] = tmpIm[k] * scale;
    }
}

/* ---- Forward FFT ---- */

QVector<double> RaderFFT7::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = (2 * i + 1 < input.size()) ? input[2 * i + 1] : 0.0;
    }

    if (m_stats.isPrime && n >= 3) {
        // Rader's algorithm: convert to circular convolution
        double x0r = re[0], x0i = im[0];

        // Permute x[1..N-1] using generator powers
        int m = n - 1;
        QVector<double> aRe(m), aIm(m);
        for (int k = 0; k < m; ++k) {
            aRe[k] = re[m_permA[k]];
            aIm[k] = im[m_permA[k]];
        }

        // Circular convolution with precomputed kernel
        QVector<double> cRe, cIm;
        const_cast<RaderFFT7*>(this)->circularConvolve(
            aRe, aIm, m_kernelRe, m_kernelIm, cRe, cIm);

        // Reassemble output
        QVector<double> out(2 * n);
        out[0] = x0r; out[1] = x0i;
        for (int k = 0; k < m; ++k) {
            int idx = m_permA[k];
            out[2 * idx] = x0r + cRe[k];
            out[2 * idx + 1] = x0i + cIm[k];
        }
        // X[0] = sum of all inputs
        double sumR = 0.0, sumI = 0.0;
        for (int i = 0; i < n; ++i) { sumR += re[i]; sumI += im[i]; }
        out[0] = sumR; out[1] = sumI;

        const_cast<RaderFFT7*>(this)->m_stats.totalOps++;
        const_cast<RaderFFT7*>(this)->m_timeSum += timer.elapsed();
        const_cast<RaderFFT7*>(this)->m_stats.avgProcessingTimeMs =
            m_timeSum / m_stats.totalOps;
        emit const_cast<RaderFFT7*>(this)->transformCompleted(n, timer.elapsed());
        return out;
    }

    // Fallback: direct DFT for non-prime sizes
    QVector<double> out(2 * n);
    for (int k = 0; k < n; ++k) {
        double sr = 0.0, si = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * j * k / n;
            sr += re[j] * qCos(angle) - im[j] * qSin(angle);
            si += re[j] * qSin(angle) + im[j] * qCos(angle);
        }
        out[2 * k] = sr;
        out[2 * k + 1] = si;
    }

    const_cast<RaderFFT7*>(this)->m_stats.totalOps++;
    const_cast<RaderFFT7*>(this)->m_timeSum += timer.elapsed();
    const_cast<RaderFFT7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<RaderFFT7*>(this)->transformCompleted(n, timer.elapsed());
    return out;
}

/* ---- Inverse FFT ---- */

QVector<double> RaderFFT7::inverse(const QVector<double>& input) const
{
    // Conjugate input, forward, conjugate output, scale by 1/N
    int n = m_n;
    QVector<double> conj(2 * n);
    for (int i = 0; i < n; ++i) {
        conj[2 * i] = (2 * i < input.size()) ? input[2 * i] : 0.0;
        conj[2 * i + 1] = (2 * i + 1 < input.size()) ? -input[2 * i + 1] : 0.0;
    }
    QVector<double> result = forward(conj);
    for (int i = 0; i < result.size(); ++i) {
        result[i] = -result[i] / n;
    }
    // Re-conjugate imaginary parts
    for (int i = 0; i < n; ++i) {
        if (2 * i + 1 < result.size())
            result[2 * i + 1] = -result[2 * i + 1];
    }
    return result;
}

/* ---- Reset ---- */

void RaderFFT7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_permA.clear();
    m_permB.clear();
    m_kernelRe.clear();
    m_kernelIm.clear();
    m_factors.clear();
}
