/**
 * @file MixedRadixFFT8.cpp
 * @brief MixedRadixFFT8 实现
 *
 * 实现混合基FFT：自动分解因子与Good排列的任意复合长度变换。
 */

#include "utils/fft247/MixedRadixFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT8::MixedRadixFFT8(QObject *parent) : QObject(parent) {}
MixedRadixFFT8::~MixedRadixFFT8() = default;

/* ---- Complex helpers (inline) ---- */

static inline double cmplRe(double a, double b, double c, double d)
{ return a * c - b * d; }

static inline double cmplIm(double a, double b, double c, double d)
{ return a * d + b * c; }

/* ---- Factorization into small radices ---- */

QVector<int> MixedRadixFFT8::factorize(int n) const
{
    QVector<int> factors;
    // Prefer radices in order: 4, 2, 3, 5, then remaining primes
    for (int r : {4, 2, 3, 5}) {
        while (n % r == 0) { factors.append(r); n /= r; }
    }
    // Remaining prime factors
    for (int p = 7; p * p <= n; p += 2) {
        while (n % p == 0) { factors.append(p); n /= p; }
    }
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Good's permutation ---- */

QVector<int> MixedRadixFFT8::goodsPermutation(int n, const QVector<int>& factors) const
{
    int r = factors.size();
    // Compute multi-radix dimensions
    QVector<int> dims(r);
    for (int i = 0; i < r; ++i) dims[i] = factors[i];

    // Compute strides for each dimension
    QVector<int> strides(r, 1);
    for (int i = r - 2; i >= 0; --i)
        strides[i] = strides[i + 1] * dims[i + 1];

    // Rader/CRT-based index mapping
    // Product of all dims = N
    int N = 1;
    for (int d : dims) N *= d;

    QVector<int> perm(N);
    for (int i = 0; i < N; ++i) {
        // Decompose i into multi-radix digits
        int idx = i;
        int mapped = 0;
        int prod = 1;
        for (int j = r - 1; j >= 0; --j) {
            int digit = idx % dims[j];
            idx /= dims[j];
            mapped += digit * strides[j];
        }
        perm[i] = mapped;
    }
    return perm;
}

/* ---- Twiddle factor ---- */

MixedRadixFFT8::Complex MixedRadixFFT8::twiddle(int k, int N, bool inv) const
{
    double angle = (inv ? 2.0 : -2.0) * M_PI * k / N;
    return {qCos(angle), qSin(angle)};
}

/* ---- Radix butterfly ---- */

void MixedRadixFFT8::radixButterfly(QVector<Complex>& data, int offset, int stride,
                                     int radix, bool inv) const
{
    // DFT of size radix on data[offset], data[offset+stride], ...
    QVector<Complex> tmp(radix);
    for (int k = 0; k < radix; ++k) {
        double reSum = 0.0, imSum = 0.0;
        for (int j = 0; j < radix; ++j) {
            int idx = offset + j * stride;
            double angle = (inv ? 2.0 : -2.0) * M_PI * k * j / radix;
            double wr = qCos(angle), wi = qSin(angle);
            reSum += data[idx].first * wr - data[idx].second * wi;
            imSum += data[idx].first * wi + data[idx].second * wr;
        }
        tmp[k] = {reSum, imSum};
    }
    for (int k = 0; k < radix; ++k)
        data[offset + k * stride] = tmp[k];
}

/* ---- Core transform ---- */

void MixedRadixFFT8::transformInternal(QVector<Complex>& data, bool inv)
{
    int N = data.size();
    if (N <= 1) return;

    m_factors = factorize(N);

    // Apply Good's permutation to reorder
    QVector<int> perm = goodsPermutation(N, m_factors);
    QVector<Complex> tmp(N);
    for (int i = 0; i < N; ++i) tmp[i] = data[perm[i]];
    data = tmp;

    // Multi-dimensional DFT via successive radix butterflies
    int stride = 1;
    for (int f : m_factors) {
        int blocks = N / (f * stride);
        for (int b = 0; b < blocks; ++b) {
            for (int s = 0; s < stride; ++s) {
                int offset = b * f * stride + s;
                radixButterfly(data, offset, stride, f, inv);
                // Apply twiddle factors between stages
                if (stride > 1) {
                    for (int k = 0; k < f; ++k) {
                        int idx = offset + k * stride;
                        auto tw = twiddle(k * (N / (f * stride)), N, inv);
                        double re = data[idx].first * tw.first - data[idx].second * tw.second;
                        double im = data[idx].first * tw.second + data[idx].second * tw.first;
                        data[idx] = {re, im};
                    }
                }
            }
        }
        stride *= f;
    }

    // Inverse permutation (unscramble)
    QVector<Complex> out(N);
    for (int i = 0; i < N; ++i) out[i] = data[perm[i]];
    data = out;

    // Scale for inverse
    if (inv) {
        for (int i = 0; i < N; ++i) {
            data[i].first /= N;
            data[i].second /= N;
        }
    }
}

/* ---- Forward (real input) ---- */

QVector<MixedRadixFFT8::Complex> MixedRadixFFT8::forward(const QVector<double>& realInput)
{
    QElapsedTimer timer;
    timer.start();

    int N = realInput.size();
    QVector<Complex> data(N);
    for (int i = 0; i < N; ++i) data[i] = {realInput[i], 0.0};

    transformInternal(data, false);

    m_stats.transformSize = N;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(N, timer.elapsed());
    return data;
}

/* ---- Forward (complex input) ---- */

QVector<MixedRadixFFT8::Complex> MixedRadixFFT8::forwardComplex(const QVector<Complex>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Complex> data = input;
    transformInternal(data, false);

    m_stats.transformSize = data.size();
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(data.size(), timer.elapsed());
    return data;
}

/* ---- Inverse ---- */

QVector<MixedRadixFFT8::Complex> MixedRadixFFT8::inverse(const QVector<Complex>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Complex> data = spectrum;
    transformInternal(data, true);

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(data.size(), timer.elapsed());
    return data;
}

/* ---- Last factorization ---- */

QVector<int> MixedRadixFFT8::lastFactorization() const { return m_factors; }

/* ---- Reset ---- */

void MixedRadixFFT8::resetStatistics()
{
    m_factors.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
