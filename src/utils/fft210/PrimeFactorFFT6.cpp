/**
 * @file PrimeFactorFFT6.cpp
 * @brief PrimeFactorFFT6 实现
 *
 * 实现素因子FFT：分裂向量嵌套、自排序原位置换、无twiddle因子DFT。
 */

#include "utils/fft210/PrimeFactorFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT6::PrimeFactorFFT6(QObject *parent) : QObject(parent) {}
PrimeFactorFFT6::~PrimeFactorFFT6() = default;

/* ---- Complex multiply ---- */

QPair<double, double> PrimeFactorFFT6::cmul(const QPair<double, double>& a,
                                              const QPair<double, double>& b)
{
    return {a.first * b.first - a.second * b.second,
            a.first * b.second + a.second * b.first};
}

/* ---- Factorize into pairwise coprime factors ---- */

QVector<int> PrimeFactorFFT6::factorize(int N)
{
    // Small pairwise coprime factors: 2, 3, 5, 7, 11, 13, ...
    static const int primes[] = {2, 3, 5, 7, 11, 13, 17, 19};
    QVector<int> factors;
    int remaining = N;

    for (int p : primes) {
        while (remaining > 1 && remaining % p == 0) {
            // Check coprimality with existing factors
            bool coprime = true;
            int temp = remaining;
            while (temp % p == 0) temp /= p;

            // Accumulate powers of p
            int pp = 1;
            while (remaining % p == 0) {
                pp *= p;
                remaining /= p;
            }
            if (pp > 1) factors.append(pp);
        }
        if (remaining == 1) break;
    }
    if (remaining > 1) factors.append(remaining);

    return factors;
}

/* ---- Check valid PFA length ---- */

bool PrimeFactorFFT6::isValidLength(int N)
{
    if (N <= 1) return false;
    auto f = factorize(N);
    int prod = 1;
    for (int v : f) prod *= v;
    return prod == N && f.size() >= 1;
}

/* ---- Next valid length ---- */

int PrimeFactorFFT6::nextValidLength(int N)
{
    for (int i = N; i <= N * 4; ++i) {
        if (isValidLength(i)) return i;
    }
    return N;
}

/* ---- CRT index mapping ---- */

int PrimeFactorFFT6::crtIndex(int i, const QVector<int>& factors, int N)
{
    Q_UNUSED(N)
    int k = factors.size();
    // Map linear index to multi-dimensional via mixed-radix
    int result = 0;
    int stride = 1;
    for (int d = 0; d < k; ++d) {
        int coord = i % factors[d];
        result += coord * stride;
        stride *= factors[d];
        i /= factors[d];
    }
    return result;
}

/* ---- Inverse CRT mapping ---- */

int PrimeFactorFFT6::inverseCrtIndex(int i, const QVector<int>& factors, int N)
{
    Q_UNUSED(N)
    int k = factors.size();
    int result = 0;
    int stride = 1;
    for (int d = k - 1; d >= 0; --d) {
        int coord = i % factors[d];
        result += coord * stride;
        stride *= factors[d];
        i /= factors[d];
    }
    return result;
}

/* ---- Small-N DFT ---- */

void PrimeFactorFFT6::smallDFT(QVector<QPair<double, double>>& data,
                                 int start, int stride, int len, bool inverse)
{
    double sign = inverse ? 1.0 : -1.0;
    QVector<QPair<double, double>> temp(len);

    for (int k = 0; k < len; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < len; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / len;
            double cs = qCos(angle), sn = qSin(angle);
            int idx = start + n * stride;
            re += data[idx].first * cs - data[idx].second * sn;
            im += data[idx].first * sn + data[idx].second * cs;
        }
        temp[k] = {re, im};
    }

    for (int k = 0; k < len; ++k)
        data[start + k * stride] = temp[k];
}

/* ---- Self-sorting in-place permutation ---- */

void PrimeFactorFFT6::selfSortPermute(QVector<QPair<double, double>>& data,
                                        const QVector<int>& factors, int N)
{
    int k = factors.size();
    if (k <= 1) return;

    QVector<bool> visited(N, false);

    for (int i = 0; i < N; ++i) {
        if (visited[i]) continue;
        int j = crtIndex(i, factors, N);
        while (j != i) {
            std::swap(data[i], data[j]);
            visited[j] = true;
            j = crtIndex(j, factors, N);
        }
        visited[i] = true;
    }
}

/* ---- Forward complex ---- */

QVector<QPair<double, double>> PrimeFactorFFT6::forwardComplex(
    const QVector<QPair<double, double>>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N <= 1) return input;

    // Find valid length
    int M = isValidLength(N) ? N : nextValidLength(N);

    QVector<QPair<double, double>> data(M, {0.0, 0.0});
    for (int i = 0; i < N; ++i) data[i] = input[i];

    auto factors = factorize(M);
    int k = factors.size();

    // Split-vector nesting: iterate over factor dimensions
    int stride = 1;
    for (int d = 0; d < k; ++d) {
        int f = factors[d];
        int outerStride = stride * f;

        // Apply small-N DFT along dimension d
        for (int base = 0; base < M; base += outerStride) {
            for (int offset = 0; offset < stride; ++offset) {
                smallDFT(data, base + offset, stride, f, false);
            }
        }

        // Self-sorting permutation after each stage
        selfSortPermute(data, factors, M);
        stride = outerStride;
    }

    data.resize(N);

    auto self = const_cast<PrimeFactorFFT6*>(this);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = N;
    self->m_stats.numFactors = k;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(N, k, timer.elapsed());

    return data;
}

/* ---- Forward real ---- */

QVector<QPair<double, double>> PrimeFactorFFT6::forward(
    const QVector<double>& input) const
{
    QVector<QPair<double, double>> complexInput(input.size());
    for (int i = 0; i < input.size(); ++i)
        complexInput[i] = {input[i], 0.0};
    return forwardComplex(complexInput);
}

/* ---- Inverse ---- */

QVector<QPair<double, double>> PrimeFactorFFT6::inverse(
    const QVector<QPair<double, double>>& spectrum) const
{
    QVector<QPair<double, double>> conjSpec(spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i)
        conjSpec[i] = {spectrum[i].first, -spectrum[i].second};

    auto result = const_cast<PrimeFactorFFT6*>(this)->forwardComplex(conjSpec);
    for (auto& z : result)
        z = {z.first / result.size(), -z.second / result.size()};
    return result;
}

/* ---- Reset ---- */

void PrimeFactorFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
