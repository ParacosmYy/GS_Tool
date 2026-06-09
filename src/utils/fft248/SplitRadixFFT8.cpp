/**
 * @file SplitRadixFFT8.cpp
 * @brief SplitRadixFFT8 实现
 *
 * 实现分裂基FFT：共轭对旋转因子优化与分裂基蝶形核。
 */

#include "utils/fft248/SplitRadixFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT8::SplitRadixFFT8(QObject *parent) : QObject(parent) {}
SplitRadixFFT8::~SplitRadixFFT8() = default;

/* ---- Compute twiddle factors with conjugate symmetry ---- */

QVector<SplitRadixFFT8::Complex> SplitRadixFFT8::computeTwiddles(int N) const
{
    // Only need N/4 unique twiddles; conjugate pair gives N/2
    int halfN = N / 2;
    QVector<Complex> tw(halfN);
    for (int k = 0; k < halfN; ++k) {
        double angle = -2.0 * M_PI * k / N;
        tw[k] = {qCos(angle), qSin(angle)};
    }
    return tw;
}

/* ---- Bit-reversal permutation ---- */

void SplitRadixFFT8::bitReverse(QVector<Complex>& data) const
{
    int N = data.size();
    int bits = 0;
    int tmp = N;
    while (tmp > 1) { bits++; tmp >>= 1; }

    for (int i = 0; i < N; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        if (rev > i) {
            Complex t = data[i];
            data[i] = data[rev];
            data[rev] = t;
        }
    }
}

/* ---- Length-2 DFT kernel ---- */

void SplitRadixFFT8::dft2(Complex& x0, Complex& x1) const
{
    double r0 = x0.first, i0 = x0.second;
    double r1 = x1.first, i1 = x1.second;
    x0 = {r0 + r1, i0 + i1};
    x1 = {r0 - r1, i0 - i1};
}

/* ---- Length-4 DFT kernel ---- */

void SplitRadixFFT8::dft4(Complex& x0, Complex& x1,
                            Complex& x2, Complex& x3) const
{
    // Stage 1: two length-2 DFTs
    double a0r = x0.first + x2.first, a0i = x0.second + x2.second;
    double a1r = x1.first + x3.first, a1i = x1.second + x3.second;
    double b0r = x0.first - x2.first, b0i = x0.second - x2.second;
    double b1r = x1.first - x3.first, b1i = x1.second - x3.second;

    // Stage 2: combine with twiddle (j multiplication for odd-index)
    x0 = {a0r + a1r, a0i + a1i};
    x2 = {a0r - a1r, a0i - a1i};
    x1 = {b0r + b1i, b0i - b1r};  // Multiply x3 by j: (r, i) -> (i, -r)
    x3 = {b0r - b1i, b0i + b1r};
}

/* ---- Recursive split-radix butterfly ---- */

void SplitRadixFFT8::butterfly(QVector<Complex>& data, int offset, int n,
                                int stride,
                                const QVector<Complex>& twiddles) const
{
    if (n <= 1) return;
    if (n == 2) {
        dft2(data[offset], data[offset + stride]);
        return;
    }
    if (n == 4) {
        dft4(data[offset], data[offset + stride],
             data[offset + 2 * stride], data[offset + 3 * stride]);
        return;
    }

    // Split: even part of size n/2, odd part split into two n/4 parts
    int n2 = n / 2;
    int n4 = n / 4;

    // Recurse on even-indexed elements (size n/2)
    butterfly(data, offset, n2, 2 * stride, twiddles);

    // Recurse on odd/4 elements (size n/4)
    butterfly(data, offset + stride, n4, 4 * stride, twiddles);

    // Recurse on odd*3/4 elements (size n/4)
    butterfly(data, offset + 3 * stride, n4, 4 * stride, twiddles);

    // Combine with twiddle factors (conjugate-pair optimization)
    for (int k = 0; k < n4; ++k) {
        int idx1 = offset + (2 * k + 1) * stride;
        int idx3 = offset + (2 * k + 3) * stride;
        int twIdx = k * stride;

        // Conjugate pair: W^k and W^{-k} = conj(W^k)
        double wr = twiddles[twIdx].first;
        double wi = twiddles[twIdx].second;

        // W^3k = W^k * W^{2k}
        int twIdx3 = 3 * twIdx;
        double wr3, wi3;
        if (twIdx3 < twiddles.size()) {
            wr3 = twiddles[twIdx3].first;
            wi3 = twiddles[twIdx3].second;
        } else {
            // Use conjugate symmetry
            int mirror = twiddles.size() * 2 - twIdx3;
            if (mirror >= 0 && mirror < twiddles.size()) {
                wr3 = twiddles[mirror].first;
                wi3 = -twiddles[mirror].second;
            } else {
                double angle = -2.0 * M_PI * twIdx3 / (2 * twiddles.size());
                wr3 = qCos(angle);
                wi3 = qSin(angle);
            }
        }

        double x1r = data[idx1].first, x1i = data[idx1].second;
        double x3r = data[idx3].first, x3i = data[idx3].second;

        // Apply twiddles
        double t1r = x1r * wr - x1i * wi;
        double t1i = x1r * wi + x1i * wr;
        double t3r = x3r * wr3 - x3i * wi3;
        double t3i = x3r * wi3 + x3i * wr3;

        int evenIdx = offset + 2 * k * stride;
        double er = data[evenIdx].first, ei = data[evenIdx].second;

        data[evenIdx] = {er + t1r + t3r, ei + t1i + t3i};
        data[idx1]    = {er - t3i + t1i, ei + t3r - t1r};
        data[evenIdx + n2 * stride] = {er - t1r - t3r, ei - t1i - t3i};
        data[idx3]    = {er + t3i - t1i, ei - t3r - t1r};
    }
}

/* ---- Core split-radix transform ---- */

void SplitRadixFFT8::splitRadixCore(QVector<Complex>& data, int N,
                                      const QVector<Complex>& twiddles) const
{
    // Bit-reverse reorder
    bitReverse(data);

    // Bottom-up iterative approach for N=4^k
    // Process L=4,16,64,... blocks using split-radix butterfly
    butterfly(data, 0, N, 1, twiddles);
}

/* ---- Forward (real input) ---- */

QVector<SplitRadixFFT8::Complex> SplitRadixFFT8::forward(
    const QVector<double>& realInput)
{
    QElapsedTimer timer;
    timer.start();

    int N = realInput.size();
    if (N <= 1) {
        QVector<Complex> r(N);
        if (N == 1) r[0] = {realInput[0], 0.0};
        return r;
    }

    // Pad to next power of 2
    int N2 = 1;
    while (N2 < N) N2 <<= 1;

    QVector<Complex> data(N2, {0.0, 0.0});
    for (int i = 0; i < N; ++i) data[i] = {realInput[i], 0.0};

    auto tw = computeTwiddles(N2);
    splitRadixCore(data, N2, tw);

    m_stats.transformSize = N;
    m_stats.numTransforms++;
    m_stats.radixSplits++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(N, timer.elapsed());
    return data;
}

/* ---- Forward (complex input) ---- */

QVector<SplitRadixFFT8::Complex> SplitRadixFFT8::forwardComplex(
    const QVector<Complex>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N <= 1) return input;

    int N2 = 1;
    while (N2 < N) N2 <<= 1;

    QVector<Complex> data(N2, {0.0, 0.0});
    for (int i = 0; i < N; ++i) data[i] = input[i];

    auto tw = computeTwiddles(N2);
    splitRadixCore(data, N2, tw);

    m_stats.transformSize = N;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(N, timer.elapsed());
    return data;
}

/* ---- Inverse ---- */

QVector<SplitRadixFFT8::Complex> SplitRadixFFT8::inverse(
    const QVector<Complex>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int N = spectrum.size();
    if (N <= 1) return spectrum;

    int N2 = 1;
    while (N2 < N) N2 <<= 1;

    // Conjugate input for inverse
    QVector<Complex> data(N2, {0.0, 0.0});
    for (int i = 0; i < N; ++i)
        data[i] = {spectrum[i].first, -spectrum[i].second};

    auto tw = computeTwiddles(N2);
    splitRadixCore(data, N2, tw);

    // Scale and conjugate back
    for (int i = 0; i < N2; ++i) {
        data[i].first /= N2;
        data[i].second = -data[i].second / N2;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(N, timer.elapsed());
    return data;
}

/* ---- Reset ---- */

void SplitRadixFFT8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
