/**
 * @file RaderFFT6.cpp
 * @brief RaderFFT6 实现
 *
 * 实现Rader FFT：素数检测、本原根、Rader素数DFT、Bluestein啁啾预计算。
 */

#include "utils/fft208/RaderFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT6::RaderFFT6(QObject *parent) : QObject(parent) {}
RaderFFT6::~RaderFFT6() = default;

/* ---- Complex helpers ---- */

QPair<double, double> RaderFFT6::cmul(const QPair<double, double>& a,
                                        const QPair<double, double>& b)
{
    return {a.first * b.first - a.second * b.second,
            a.first * b.second + a.second * b.first};
}

/* ---- Prime check ---- */

bool RaderFFT6::isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

/* ---- Primitive root ---- */

int RaderFFT6::primitiveRoot(int p)
{
    if (p == 2) return 1;
    // Factor p-1
    int phi = p - 1;
    QVector<int> factors;
    int temp = phi;
    for (int i = 2; i * i <= temp; ++i) {
        if (temp % i == 0) {
            factors.append(i);
            while (temp % i == 0) temp /= i;
        }
    }
    if (temp > 1) factors.append(temp);

    for (int g = 2; g < p; ++g) {
        bool ok = true;
        for (int q : factors) {
            long long val = 1;
            for (int i = 0; i < phi / q; ++i) val = (val * g) % p;
            if (val == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return -1;
}

/* ---- Bit reverse ---- */

int RaderFFT6::bitReverse(int x, int bits)
{
    int r = 0;
    for (int i = 0; i < bits; ++i) { r = (r << 1) | (x & 1); x >>= 1; }
    return r;
}

/* ---- Next power of 2 ---- */

int RaderFFT6::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- In-place radix-2 FFT ---- */

void RaderFFT6::fftImpl(QVector<QPair<double, double>>& data, bool inverse)
{
    int N = data.size();
    if (N <= 1) return;

    int bits = 0;
    while ((1 << bits) < N) ++bits;

    for (int i = 0; i < N; ++i) {
        int j = bitReverse(i, bits);
        if (j > i) std::swap(data[i], data[j]);
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        QPair<double, double> wn{qCos(angle), qSin(angle)};
        for (int i = 0; i < N; i += len) {
            QPair<double, double> w{1.0, 0.0};
            for (int j = 0; j < len / 2; ++j) {
                auto u = data[i + j];
                auto v = cmul(w, data[i + j + len / 2]);
                data[i + j] = {u.first + v.first, u.second + v.second};
                data[i + j + len / 2] = {u.first - v.first, u.second - v.second};
                w = cmul(w, wn);
            }
        }
    }

    if (inverse) {
        for (auto& z : data) { z.first /= N; z.second /= N; }
    }
}

/* ---- Precompute chirp for Rader ---- */

void RaderFFT6::precomputeChirp(int primeN)
{
    if (!isPrime(primeN)) return;
    m_cachedPrime = primeN;
    int p = primeN;
    int g = primitiveRoot(p);

    // Build permutation from primitive root
    m_permForward.resize(p - 1);
    m_permInverse.resize(p - 1);
    long long val = 1;
    for (int i = 0; i < p - 1; ++i) {
        m_permForward[i] = static_cast<int>(val);
        m_permInverse[static_cast<int>(val) - 1] = i;
        val = (val * g) % p;
    }

    // Chirp sequence: W_N^(g^(-i)) for i = 0..p-2
    m_chirpSeq.resize(p - 1);
    m_chirpConjSeq.resize(p - 1);
    for (int i = 0; i < p - 1; ++i) {
        double angle = 2.0 * M_PI * m_permForward[i] / p;
        m_chirpSeq[i] = {qCos(angle), qSin(angle)};
        m_chirpConjSeq[i] = {qCos(-angle), qSin(-angle)};
    }
}

/* ---- Rader prime DFT ---- */

QVector<QPair<double, double>> RaderFFT6::raderPrime(
    const QVector<QPair<double, double>>& input) const
{
    int p = input.size();
    int n = p - 1;
    int L = nextPow2(n);

    // Extract DC
    QPair<double, double> dc = {0.0, 0.0};
    for (int i = 0; i < p; ++i) {
        dc.first += input[i].first;
        dc.second += input[i].second;
    }

    // Permute input and multiply by chirp
    QVector<QPair<double, double>> a(L, {0.0, 0.0});
    QVector<QPair<double, double>> b(L, {0.0, 0.0});

    for (int i = 0; i < n; ++i) {
        a[i] = cmul(input[m_permForward[i]], m_chirpConjSeq[i]);
        b[i] = m_chirpSeq[i];
    }

    // Convolution via FFT
    fftImpl(a, false);
    fftImpl(b, false);
    for (int i = 0; i < L; ++i)
        a[i] = cmul(a[i], b[i]);
    fftImpl(a, true);

    // Build output
    QVector<QPair<double, double>> output(p);
    output[0] = dc;
    for (int i = 0; i < n; ++i)
        output[m_permForward[i]] = cmul(a[i], m_chirpConjSeq[i]);

    return output;
}

/* ---- Forward complex ---- */

QVector<QPair<double, double>> RaderFFT6::forwardComplex(
    const QVector<QPair<double, double>>& input) const
{
    QElapsedTimer timer;
    timer.start();
    int N = input.size();
    QVector<QPair<double, double>> output;

    if (isPrime(N) && N > 16) {
        // Use Rader's algorithm for prime sizes
        RaderFFT6* self = const_cast<RaderFFT6*>(this);
        if (self->m_cachedPrime != N)
            self->precomputeChirp(N);
        output = raderPrime(input);
        self->m_stats.lastWasPrime = true;
    } else if ((N & (N - 1)) == 0) {
        // Power of 2: use radix-2
        output = input;
        fftImpl(output, false);
        const_cast<RaderFFT6*>(this)->m_stats.lastWasPrime = false;
    } else {
        // Composite non-power-of-2: use Bluestein
        int M = N;
        int L = nextPow2(2 * N - 1);
        QVector<QPair<double, double>> chirp(L, {0.0, 0.0});
        for (int k = 0; k < N; ++k) {
            double angle = M_PI * k * k / N;
            chirp[k] = {qCos(angle), qSin(angle)};
        }
        for (int k = N; k < L - N + 1; ++k)
            chirp[k] = {0.0, 0.0};
        for (int k = L - N + 1; k < L; ++k) {
            int idx = N - 1 - (L - k);
            double angle = M_PI * idx * idx / N;
            chirp[k] = {qCos(angle), qSin(angle)};
        }

        QVector<QPair<double, double>> y(L, {0.0, 0.0});
        for (int i = 0; i < N; ++i)
            y[i] = cmul(input[i], {chirp[i].first, -chirp[i].second});

        fftImpl(chirp, false);
        fftImpl(y, false);
        for (int i = 0; i < L; ++i)
            y[i] = cmul(y[i], chirp[i]);
        fftImpl(y, true);

        output.resize(N);
        for (int k = 0; k < N; ++k)
            output[k] = cmul(y[k], {chirp[k].first, -chirp[k].second});
        const_cast<RaderFFT6*>(this)->m_stats.lastWasPrime = false;
    }

    const_cast<RaderFFT6*>(this)->m_stats.totalTransforms++;
    const_cast<RaderFFT6*>(this)->m_stats.lastSize = N;
    const_cast<RaderFFT6*>(this)->m_timeSum += timer.elapsed();
    const_cast<RaderFFT6*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalTransforms;
    const_cast<RaderFFT6*>(this)->emit transformCompleted(
        N, m_stats.lastWasPrime, timer.elapsed());

    return output;
}

/* ---- Forward real ---- */

QVector<QPair<double, double>> RaderFFT6::forward(const QVector<double>& input) const
{
    QVector<QPair<double, double>> complexInput(input.size());
    for (int i = 0; i < input.size(); ++i)
        complexInput[i] = {input[i], 0.0};
    return forwardComplex(complexInput);
}

/* ---- Inverse ---- */

QVector<QPair<double, double>> RaderFFT6::inverse(
    const QVector<QPair<double, double>>& spectrum) const
{
    QVector<QPair<double, double>> conjSpec(spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i)
        conjSpec[i] = {spectrum[i].first, -spectrum[i].second};

    auto result = const_cast<RaderFFT6*>(this)->forwardComplex(conjSpec);
    for (auto& z : result)
        z = {z.first / result.size(), -z.second / result.size()};
    return result;
}

/* ---- Reset ---- */

void RaderFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_chirpSeq.clear();
    m_chirpConjSeq.clear();
    m_permForward.clear();
    m_permInverse.clear();
    m_cachedPrime = 0;
}
