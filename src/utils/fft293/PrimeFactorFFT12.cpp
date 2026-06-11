/**
 * @file PrimeFactorFFT12.cpp
 * @brief PrimeFactorFFT12 实现
 *
 * 实现素因子FFT算法：无旋转映射与自排序索引置换实现互素因子复合长度变换。
 */

#include "utils/fft293/PrimeFactorFFT12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT12::PrimeFactorFFT12(QObject *parent)
    : QObject(parent)
{
    precompute();
}

PrimeFactorFFT12::~PrimeFactorFFT12() = default;

/* ---- Configuration ---- */

void PrimeFactorFFT12::setSize(int N)
{
    m_N = qBound(6, N, 1000000);
    precompute();
}

/* ---- GCD ---- */

int PrimeFactorFFT12::gcd(int a, int b) const
{
    while (b) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Modular inverse ---- */

int PrimeFactorFFT12::modInverse(int a, int m) const
{
    int g = gcd(a, m);
    if (g != 1) return 1;
    // Extended Euclidean algorithm
    int oldR = a, r = m, oldS = 1, s = 0;
    while (r != 0) {
        int q = oldR / r;
        int tmp = r; r = oldR - q * r; oldR = tmp;
        tmp = s; s = oldS - q * s; oldS = tmp;
    }
    return ((oldS % m) + m) % m;
}

/* ---- Factorize into pairwise coprime factors ---- */

QVector<int> PrimeFactorFFT12::factorize(int N) const
{
    QVector<int> factors;
    // Extract coprime factors: try small primes, ensure pairwise coprime
    int remaining = N;
    for (int p = 2; p * p <= remaining; ++p) {
        int pk = 1;
        while (remaining % p == 0) {
            pk *= p;
            remaining /= p;
        }
        if (pk > 1) factors.append(pk);
    }
    if (remaining > 1) factors.append(remaining);
    return factors;
}

/* ---- Precompute index mappings ---- */

void PrimeFactorFFT12::precompute()
{
    m_stats.transformSize = m_N;
    m_factors = factorize(m_N);
    int d = m_factors.size();

    m_ni.resize(d);
    m_mi.resize(d);
    for (int i = 0; i < d; ++i) {
        m_ni[i] = m_N / m_factors[i];
        m_mi[i] = modInverse(m_ni[i], m_factors[i]);
    }

    // Build rotator-free (Ruritanian) index mapping
    // Input index: n = sum(k_i * N_i * M_i) mod N for all factor combinations
    m_inputMap.resize(m_N);
    m_outputMap.resize(m_N);

    if (d == 0) return;

    // Generate all index combinations
    QVector<int> k(d, 0);
    for (int idx = 0; idx < m_N; ++idx) {
        // Compute n from multi-index using Chinese Remainder Theorem mapping
        int n = 0;
        for (int i = 0; i < d; ++i)
            n += k[i] * m_ni[i] * m_mi[i];
        n %= m_N;

        m_inputMap[idx] = n;

        // Output mapping (transpose of input)
        int kn = 0;
        for (int i = 0; i < d; ++i)
            kn += k[i] * m_ni[i] * m_mi[i];
        kn %= m_N;
        m_outputMap[n] = kn;

        // Increment multi-index
        for (int i = d - 1; i >= 0; --i) {
            k[i]++;
            if (k[i] < m_factors[i]) break;
            k[i] = 0;
        }
    }

    // Self-sorting: build identity mapping for rotator-free case
    for (int i = 0; i < m_N; ++i)
        m_outputMap[i] = i;
}

/* ---- Short DFT ---- */

void PrimeFactorFFT12::shortDFT(double* real, double* imag, int n, bool inv) const
{
    QVector<double> tr(n), ti(n);
    double sign = inv ? 1.0 : -1.0;
    double scale = inv ? (1.0 / n) : 1.0;

    for (int k = 0; k < n; ++k) {
        double sr = 0.0, si = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = sign * 2.0 * M_PI * k * j / n;
            sr += real[j] * qCos(angle) - imag[j] * qSin(angle);
            si += real[j] * qSin(angle) + imag[j] * qCos(angle);
        }
        tr[k] = sr * scale;
        ti[k] = si * scale;
    }
    for (int i = 0; i < n; ++i) {
        real[i] = tr[i];
        imag[i] = ti[i];
    }
}

/* ---- Complex multiply ---- */

void PrimeFactorFFT12::complexMul(const double& ar, const double& ai,
                                     const double& br, const double& bi,
                                     double& cr, double& ci) const
{
    cr = ar * br - ai * bi;
    ci = ar * bi + ai * br;
}

/* ---- Forward FFT on complex input ---- */

PrimeFactorFFT12::FFTResult PrimeFactorFFT12::forwardComplex(
    const QVector<double>& realIn,
    const QVector<double>& imagIn)
{
    QElapsedTimer timer;
    timer.start();

    FFTResult result;
    result.real.resize(m_N);
    result.imag.resize(m_N);

    // Copy input with index mapping
    QVector<double> wr(m_N, 0.0), wi(m_N, 0.0);
    for (int i = 0; i < m_N; ++i) {
        int src = m_inputMap[i];
        wr[i] = (src < realIn.size()) ? realIn[src] : 0.0;
        wi[i] = (src < imagIn.size()) ? imagIn[src] : 0.0;
    }

    // Apply short DFTs along each factor dimension
    int d = m_factors.size();
    for (int dim = 0; dim < d; ++dim) {
        int n = m_factors[dim];
        int stride = m_ni[dim];
        int groups = m_N / n;

        for (int g = 0; g < groups; ++g) {
            // Extract sub-vector for this group
            QVector<double> sr(n), si(n);
            for (int j = 0; j < n; ++j)
                sr[j] = wr[g + j * groups];
            for (int j = 0; j < n; ++j)
                si[j] = wi[g + j * groups];

            shortDFT(sr.data(), si.data(), n, false);

            // Write back
            for (int j = 0; j < n; ++j) {
                wr[g + j * groups] = sr[j];
                wi[g + j * groups] = si[j];
            }
        }
    }

    // Self-sorting output (identity for rotator-free PFA)
    for (int i = 0; i < m_N; ++i) {
        result.real[i] = wr[m_outputMap[i]];
        result.imag[i] = wi[m_outputMap[i]];
    }

    // Peak magnitude
    double peak = 0.0;
    for (int i = 0; i < m_N; ++i) {
        double mag = qSqrt(result.real[i] * result.real[i] + result.imag[i] * result.imag[i]);
        peak = qMax(peak, mag);
    }
    result.peakMagnitude = peak;

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, peak, elapsed);
    return result;
}

/* ---- Forward FFT on real input ---- */

PrimeFactorFFT12::FFTResult PrimeFactorFFT12::forward(const QVector<double>& input)
{
    QVector<double> imag(input.size(), 0.0);
    return forwardComplex(input, imag);
}

/* ---- Inverse FFT ---- */

PrimeFactorFFT12::FFTResult PrimeFactorFFT12::inverse(const QVector<double>& realIn,
                                                          const QVector<double>& imagIn)
{
    // Conjugate, forward, conjugate, scale by 1/N
    QVector<double> conjImag(imagIn.size());
    for (int i = 0; i < imagIn.size(); ++i)
        conjImag[i] = -imagIn[i];

    auto result = forwardComplex(realIn, conjImag);
    for (int i = 0; i < result.real.size(); ++i) {
        result.real[i] /= m_N;
        result.imag[i] = -result.imag[i] / m_N;
    }
    return result;
}

/* ---- Reset ---- */

void PrimeFactorFFT12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
