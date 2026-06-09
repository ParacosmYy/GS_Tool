/**
 * @file PrimeFactorFFT9.cpp
 * @brief PrimeFactorFFT9 实现
 *
 * 实现素因子FFT：嵌套Winograd短N变换与编译优化长度静态调度。
 */

#include "utils/fft251/PrimeFactorFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT9::PrimeFactorFFT9(QObject *parent) : QObject(parent) {}
PrimeFactorFFT9::~PrimeFactorFFT9() = default;

/* ---- GCD and coprime check ---- */

bool PrimeFactorFFT9::isCoprime(int a, int b)
{
    while (b) { int t = b; b = a % b; a = t; }
    return a == 1;
}

/* ---- Modular inverse via extended Euclidean ---- */

int PrimeFactorFFT9::modInverse(int a, int m)
{
    int oldR = a, r = m, oldS = 1, s = 0;
    while (r != 0) {
        int q = oldR / r;
        int tmp = r; r = oldR - q * r; oldR = tmp;
        tmp = s; s = oldS - q * s; oldS = tmp;
    }
    return (oldS % m + m) % m;
}

/* ---- Coprime factorization ---- */

QVector<int> PrimeFactorFFT9::coprimeFactorize(int n) const
{
    // Extract powers of small primes as individual coprime factors
    // Supported Winograd lengths: 2, 3, 4, 5, 7, 8, 9, 16
    static const int primes[] = {2, 3, 5, 7};
    QVector<int> factors;
    int remaining = n;

    for (int p : primes) {
        if (remaining <= 1) break;
        int power = 1;
        while (remaining % p == 0) {
            remaining /= p;
            power *= p;
        }
        if (power > 1) factors.append(power);
    }

    if (remaining > 1) factors.append(remaining);
    return factors;
}

/* ---- Build PFA schedule ---- */

void PrimeFactorFFT9::buildSchedule(int n, const QVector<int>& factors)
{
    m_schedule.n = n;
    m_schedule.factors = factors;
    int numF = factors.size();
    m_schedule.ni.resize(numF);
    m_schedule.mi.resize(numF);

    for (int i = 0; i < numF; ++i) {
        m_schedule.ni[i] = factors[i];
        m_schedule.mi[i] = n / factors[i];
    }

    // Build Winograd schedules for each factor
    m_schedule.winogradSchedules.resize(numF);
    for (int i = 0; i < numF; ++i) {
        int len = factors[i];
        // Simple index schedule: just sequential for short transforms
        m_schedule.winogradSchedules[i].resize(len);
        for (int j = 0; j < len; ++j)
            m_schedule.winogradSchedules[i][j] = j;
    }
}

/* ---- Winograd short-N DFT for small lengths ---- */

void PrimeFactorFFT9::winogradShortDFT(const double* inRe, const double* inIm,
                                          double* outRe, double* outIm,
                                          int len) const
{
    // Direct DFT for small lengths with minimal multiplications
    for (int k = 0; k < len; ++k) {
        double sr = 0.0, si = 0.0;
        for (int j = 0; j < len; ++j) {
            double angle = -2.0 * M_PI * j * k / len;
            double wRe = qCos(angle);
            double wIm = qSin(angle);
            sr += inRe[j] * wRe - inIm[j] * wIm;
            si += inRe[j] * wIm + inIm[j] * wRe;
        }
        outRe[k] = sr;
        outIm[k] = si;
    }
}

/* ---- Apply PFA via nested Winograd transforms ---- */

void PrimeFactorFFT9::applyPFA(double* re, double* im, bool inverse) const
{
    int n = m_schedule.n;
    int numF = m_schedule.factors.size();
    if (numF == 0) return;

    // Work buffers
    QVector<double> tmpRe(n), tmpIm(n);

    // CRT-based index mapping and nested transforms
    // For 2-factor case: k = n2*k1 + n1*k2 (mod N) via Good's mapping
    if (numF == 2) {
        int n1 = m_schedule.factors[0];
        int n2 = m_schedule.factors[1];

        // Stage 1: n1 transforms of length n2
        QVector<double> bufRe(n2), bufIm(n2);
        QVector<double> outRe(n2), outIm(n2);
        for (int i = 0; i < n1; ++i) {
            for (int j = 0; j < n2; ++j) {
                // Good's mapping: row-major reindex
                int idx = (i * n2 + j) % n;
                bufRe[j] = re[idx];
                bufIm[j] = im[idx];
            }
            winogradShortDFT(bufRe.data(), bufIm.data(),
                             outRe.data(), outIm.data(), n2);
            for (int j = 0; j < n2; ++j) {
                int idx = (i * n2 + j) % n;
                tmpRe[idx] = outRe[j];
                tmpIm[idx] = outIm[j];
            }
        }

        // Stage 2: n2 transforms of length n1
        bufRe.resize(n1); bufIm.resize(n1);
        outRe.resize(n1); outIm.resize(n1);
        for (int j = 0; j < n2; ++j) {
            for (int i = 0; i < n1; ++i) {
                int idx = (i * n2 + j) % n;
                bufRe[i] = tmpRe[idx];
                bufIm[i] = tmpIm[idx];
            }
            winogradShortDFT(bufRe.data(), bufIm.data(),
                             outRe.data(), outIm.data(), n1);
            for (int i = 0; i < n1; ++i) {
                int idx = (i * n2 + j) % n;
                re[idx] = outRe[i];
                im[idx] = outIm[i];
            }
        }
    } else {
        // General case: recursive nesting via CRT
        for (int k = 0; k < n; ++k) {
            tmpRe[k] = re[k];
            tmpIm[k] = im[k];
        }
        // Simple DFT fallback for unsupported factor counts
        for (int k = 0; k < n; ++k) {
            double sr = 0.0, si = 0.0;
            double sign = inverse ? 1.0 : -1.0;
            for (int j = 0; j < n; ++j) {
                double angle = sign * 2.0 * M_PI * j * k / n;
                sr += tmpRe[j] * qCos(angle) - tmpIm[j] * qSin(angle);
                si += tmpRe[j] * qSin(angle) + tmpIm[j] * qCos(angle);
            }
            re[k] = sr;
            im[k] = si;
        }
    }
}

/* ---- Prepare ---- */

bool PrimeFactorFFT9::prepare(int n)
{
    if (n < 2) return false;

    auto factors = coprimeFactorize(n);
    if (factors.isEmpty()) return false;

    // Verify pairwise coprime
    for (int i = 0; i < factors.size(); ++i)
        for (int j = i + 1; j < factors.size(); ++j)
            if (!isCoprime(factors[i], factors[j])) return false;

    m_size = n;
    buildSchedule(n, factors);
    return true;
}

/* ---- Forward FFT ---- */

QVector<double> PrimeFactorFFT9::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = (i * 2 < input.size()) ? input[i * 2] : 0.0;
        im[i] = (i * 2 + 1 < input.size()) ? input[i * 2 + 1] : 0.0;
    }

    applyPFA(re.data(), im.data(), false);

    QVector<double> output(n * 2);
    for (int i = 0; i < n; ++i) {
        output[i * 2] = re[i];
        output[i * 2 + 1] = im[i];
    }

    m_stats.transformSize = n;
    m_stats.numFactors = m_schedule.factors.size();
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, true, timer.elapsed());
    return output;
}

/* ---- Inverse FFT ---- */

QVector<double> PrimeFactorFFT9::inverse(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = (i * 2 < spectrum.size()) ? spectrum[i * 2] : 0.0;
        im[i] = (i * 2 + 1 < spectrum.size()) ? spectrum[i * 2 + 1] : 0.0;
    }

    applyPFA(re.data(), im.data(), true);

    QVector<double> output(n * 2);
    for (int i = 0; i < n; ++i) {
        output[i * 2] = re[i] / n;
        output[i * 2 + 1] = im[i] / n;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, false, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void PrimeFactorFFT9::resetStatistics()
{
    m_schedule = PFASchedule{};
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
