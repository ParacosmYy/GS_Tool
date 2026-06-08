/**
 * @file PrimeFactorFFT7.cpp
 * @brief PrimeFactorFFT7 实现
 *
 * 实现素因子FFT：无旋转因子索引映射、Winograd嵌套小N DFT。
 */

#include "utils/fft223/PrimeFactorFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT7::PrimeFactorFFT7(QObject *parent) : QObject(parent) {}
PrimeFactorFFT7::~PrimeFactorFFT7() = default;

/* ---- Extended GCD ---- */

int PrimeFactorFFT7::extGcd(int a, int b, int& x, int& y)
{
    if (b == 0) { x = 1; y = 0; return a; }
    int x1, y1;
    int g = extGcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

/* ---- Coprime factorization ---- */

QVector<int> PrimeFactorFFT7::coprimeFactors(int n)
{
    QVector<int> factors;
    // Extract factor 2
    while (n % 2 == 0) { factors.append(2); n /= 2; }
    // Extract factor 3
    while (n % 3 == 0) { factors.append(3); n /= 3; }
    // Extract factor 5
    while (n % 5 == 0) { factors.append(5); n /= 5; }
    // Extract factor 7
    while (n % 7 == 0) { factors.append(7); n /= 7; }
    // Remaining (prime > 7, or 1)
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Build Ruritanian index mapping ---- */

void PrimeFactorFFT7::buildIndexMapping()
{
    int d = m_factors.size();
    if (d == 0) return;

    m_ni = m_factors;
    m_alpha.resize(d);
    m_beta.resize(d);

    // Compute product of all factors
    // For pairwise coprime mapping: alpha[i] * N/Ni ≡ 1 (mod Ni)
    for (int i = 0; i < d; ++i) {
        int Ni = m_ni[i];
        int NoverNi = m_n / Ni;
        int x, y;
        int g = extGcd(NoverNi % Ni, Ni, x, y);
        m_alpha[i] = ((x % Ni) + Ni) % Ni;
        m_beta[i] = ((NoverNi % m_n) * m_alpha[i]) % m_n;
    }
    m_stats.isRotatorFree = true;
}

/* ---- Prepare ---- */

bool PrimeFactorFFT7::prepare(int n)
{
    if (n < 2) return false;
    m_n = n;
    m_stats.transformSize = n;

    m_factors = coprimeFactors(n);
    m_stats.numFactors = m_factors.size();

    // Verify pairwise coprime
    for (int i = 0; i < m_factors.size(); ++i) {
        for (int j = i + 1; j < m_factors.size(); ++j) {
            int a = m_factors[i], b = m_factors[j];
            // Simple GCD check
            while (b) { int t = a % b; a = b; b = t; }
            if (a != 1) {
                m_stats.isRotatorFree = false;
                return false; // Not pairwise coprime
            }
        }
    }

    buildIndexMapping();
    return true;
}

/* ---- Winograd 2-point DFT ---- */

void PrimeFactorFFT7::winograd2(double& re0, double& im0,
                                   double& re1, double& im1) const
{
    // 2-point: addition and subtraction, no multiplications
    double sr = re0 + re1, si = im0 + im1;
    double dr = re0 - re1, di = im0 - im1;
    re0 = sr; im0 = si;
    re1 = dr; im1 = di;
}

/* ---- Winograd 3-point DFT ---- */

void PrimeFactorFFT7::winograd3(double* re, double* im) const
{
    // W3 = exp(-2*pi*i/3), uses 1 complex multiply
    double c = qCos(2.0 * M_PI / 3.0);
    double s = qSin(2.0 * M_PI / 3.0);

    double a0r = re[0], a0i = im[0];
    double a1r = re[1], a1i = im[1];
    double a2r = re[2], a2i = im[2];

    // Pre-add
    double t1r = a1r + a2r, t1i = a1i + a2i;
    // Twiddle multiply (a1 - a2) * W
    double dr = a1r - a2r, di = a1i - a2i;
    double wr = dr * c - di * s;
    double wi = dr * s + di * c;

    re[0] = a0r + t1r;    im[0] = a0i + t1i;
    re[1] = a0r + t1r * c + wr; im[1] = a0i + t1i * c + wi;
    re[2] = a0r + t1r * c - wr; im[2] = a0i + t1i * c - wi;
}

/* ---- Winograd 5-point DFT ---- */

void PrimeFactorFFT7::winograd5(double* re, double* im) const
{
    // Direct 5-point DFT (Winograd optimized uses fewer multiplies)
    double sr[5], si[5];
    for (int k = 0; k < 5; ++k) {
        sr[k] = 0.0; si[k] = 0.0;
        for (int j = 0; j < 5; ++j) {
            double angle = -2.0 * M_PI * j * k / 5.0;
            double c = qCos(angle), s = qSin(angle);
            sr[k] += re[j] * c - im[j] * s;
            si[k] += re[j] * s + im[j] * c;
        }
    }
    for (int k = 0; k < 5; ++k) { re[k] = sr[k]; im[k] = si[k]; }
}

/* ---- General small-N DFT ---- */

void PrimeFactorFFT7::smallDFT(double* re, double* im, int n) const
{
    QVector<double> sr(n), si(n);
    for (int k = 0; k < n; ++k) {
        sr[k] = 0.0; si[k] = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * j * k / n;
            double c = qCos(angle), s = qSin(angle);
            sr[k] += re[j] * c - im[j] * s;
            si[k] += re[j] * s + im[j] * c;
        }
    }
    for (int k = 0; k < n; ++k) { re[k] = sr[k]; im[k] = si[k]; }
}

/* ---- Apply nested small-N DFT along one dimension ---- */

void PrimeFactorFFT7::applyNestedDFT(double* re, double* im, int dim,
                                        int factorIdx, bool inverse) const
{
    int Ni = m_ni[factorIdx];
    int prod = m_n / Ni;

    for (int k = 0; k < prod; ++k) {
        // Extract Ni elements along this dimension
        QVector<double> tmpR(Ni), tmpI(Ni);
        for (int j = 0; j < Ni; ++j) {
            int idx = (inverse) ? k * Ni + j : j * prod + k;
            if (idx < dim) { tmpR[j] = re[idx]; tmpI[j] = im[idx]; }
        }

        // Apply appropriate Winograd DFT
        switch (Ni) {
        case 2: winograd2(tmpR[0], tmpI[0], tmpR[1], tmpI[1]); break;
        case 3: winograd3(tmpR.data(), tmpI.data()); break;
        case 5: winograd5(tmpR.data(), tmpI.data()); break;
        default: smallDFT(tmpR.data(), tmpI.data(), Ni); break;
        }

        if (inverse) {
            for (int j = 0; j < Ni; ++j) {
                int idx = k * Ni + j;
                if (idx < dim) { re[idx] = tmpR[j] / Ni; im[idx] = tmpI[j] / Ni; }
            }
        } else {
            for (int j = 0; j < Ni; ++j) {
                int idx = j * prod + k;
                if (idx < dim) { re[idx] = tmpR[j]; im[idx] = tmpI[j]; }
            }
        }
    }
}

/* ---- Forward FFT ---- */

QVector<double> PrimeFactorFFT7::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = (2 * i + 1 < input.size()) ? input[2 * i + 1] : 0.0;
    }

    // Apply nested DFTs for each factor (rotator-free PFA)
    int prod = 1;
    for (int d = 0; d < m_factors.size(); ++d) {
        int Ni = m_ni[d];
        // Remap using Ruritanian indexing
        QVector<double> mappedR(n), mappedI(n);
        for (int i = 0; i < n; ++i) {
            // Compute index in factor d dimension
            int hi = i / prod;
            int lo = i % prod;
            if (hi < Ni) {
                int idx = hi * prod + lo;
                mappedR[idx] = re[i];
                mappedI[idx] = im[i];
            }
        }
        re = mappedR;
        im = mappedI;

        // Apply DFT along this dimension
        const_cast<PrimeFactorFFT7*>(this)->applyNestedDFT(
            re.data(), im.data(), n, d, false);
        prod *= Ni;
    }

    // Pack output
    QVector<double> out(2 * n);
    for (int i = 0; i < n; ++i) { out[2 * i] = re[i]; out[2 * i + 1] = im[i]; }

    const_cast<PrimeFactorFFT7*>(this)->m_stats.totalOps++;
    const_cast<PrimeFactorFFT7*>(this)->m_timeSum += timer.elapsed();
    const_cast<PrimeFactorFFT7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<PrimeFactorFFT7*>(this)->transformCompleted(n, timer.elapsed());
    return out;
}

/* ---- Inverse FFT ---- */

QVector<double> PrimeFactorFFT7::inverse(const QVector<double>& input) const
{
    int n = m_n;
    QVector<double> conj(2 * n);
    for (int i = 0; i < n; ++i) {
        conj[2 * i] = (2 * i < input.size()) ? input[2 * i] : 0.0;
        conj[2 * i + 1] = (2 * i + 1 < input.size()) ? -input[2 * i + 1] : 0.0;
    }
    QVector<double> result = forward(conj);
    for (int i = 0; i < result.size(); ++i) result[i] /= n;
    for (int i = 0; i < n && 2 * i + 1 < result.size(); ++i)
        result[2 * i + 1] = -result[2 * i + 1];
    return result;
}

/* ---- Reset ---- */

void PrimeFactorFFT7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_factors.clear();
    m_ni.clear();
    m_alpha.clear();
    m_beta.clear();
}
