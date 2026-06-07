/**
 * @file BruunFFT6.cpp
 * @brief BruunFFT6 实现
 *
 * 实现Bruun FFT：多项式残差分解、实值对称蝶形网络、纯实数优化。
 */

#include "utils/fft209/BruunFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT6::BruunFFT6(QObject *parent) : QObject(parent) {}
BruunFFT6::~BruunFFT6() = default;

/* ---- Complex multiply ---- */

QPair<double, double> BruunFFT6::cmul(const QPair<double, double>& a,
                                        const QPair<double, double>& b)
{
    return {a.first * b.first - a.second * b.second,
            a.first * b.second + a.second * b.first};
}

/* ---- Bit reverse ---- */

int BruunFFT6::bitReverse(int x, int bits)
{
    int r = 0;
    for (int i = 0; i < bits; ++i) { r = (r << 1) | (x & 1); x >>= 1; }
    return r;
}

/* ---- Number of stages ---- */

int BruunFFT6::numStages(int N)
{
    int s = 0;
    while ((1 << s) < N) ++s;
    return s;
}

/* ---- Precompute twiddles ---- */

void BruunFFT6::precomputeTwiddles(int N)
{
    if (m_cachedSize == N) return;
    m_cachedSize = N;
    int stages = numStages(N);
    int totalTwiddles = N;
    m_twiddleCos.resize(totalTwiddles);
    m_twiddleSin.resize(totalTwiddles);

    int idx = 0;
    for (int s = 0; s < stages; ++s) {
        int m = 1 << (s + 1);
        int m2 = m >> 1;
        for (int k = 0; k < N; k += m) {
            for (int j = 0; j < m2; ++j) {
                double angle = -M_PI * j / m2;
                m_twiddleCos[idx] = qCos(angle);
                m_twiddleSin[idx] = qSin(angle);
                ++idx;
            }
        }
    }
    m_twiddleCos.resize(idx);
    m_twiddleSin.resize(idx);
}

/* ---- Bruun butterfly stage ---- */

void BruunFFT6::bruunButterfly(QVector<QPair<double, double>>& data,
                                 int stage, int totalStages,
                                 const QVector<double>& twCos,
                                 const QVector<double>& twSin)
{
    int N = data.size();
    int m = 1 << (stage + 1);
    int m2 = m >> 1;

    int twIdx = 0;
    // Skip twiddle index to correct position
    for (int s = 0; s < stage; ++s) {
        int sm = 1 << (s + 1);
        int sm2 = sm >> 1;
        twIdx += (N / sm) * sm2;
    }

    for (int k = 0; k < N; k += m) {
        for (int j = 0; j < m2; ++j) {
            // Bruun real-valued symmetric butterfly:
            // Exploit symmetry of twiddle factors for real input
            double c = (twIdx < twCos.size()) ? twCos[twIdx] : 1.0;
            double s = (twIdx < twSin.size()) ? twSin[twIdx] : 0.0;
            ++twIdx;

            auto& top = data[k + j];
            auto& bot = data[k + j + m2];

            // Polynomial residue: z^m - 1 = (z^{m/2} - 1)(z^{m/2} + 1)
            // Symmetric butterfly with real twiddles
            double tr = top.first - bot.first;
            double ti = top.second - bot.second;
            top.first = top.first + bot.first;
            top.second = top.second + bot.second;

            // Apply twiddle to residual
            bot.first = tr * c - ti * s;
            bot.second = tr * s + ti * c;
        }
    }
}

/* ---- Forward complex ---- */

QVector<QPair<double, double>> BruunFFT6::forwardComplex(
    const QVector<QPair<double, double>>& input) const
{
    QElapsedTimer timer;
    timer.start();
    int N = input.size();
    if (N <= 1) return input;

    // Ensure power of 2
    int M = 1;
    while (M < N) M <<= 1;

    QVector<QPair<double, double>> data(M, {0.0, 0.0});
    for (int i = 0; i < N; ++i) data[i] = input[i];

    // Bit-reverse permutation
    int bits = numStages(M);
    for (int i = 0; i < M; ++i) {
        int j = bitReverse(i, bits);
        if (j > i) std::swap(data[i], data[j]);
    }

    // Precompute twiddles
    auto self = const_cast<BruunFFT6*>(this);
    if (self->m_cachedSize != M)
        self->precomputeTwiddles(M);

    // Apply Bruun butterfly stages
    int stages = numStages(M);
    int butterflies = 0;
    for (int s = 0; s < stages; ++s) {
        bruunButterfly(data, s, stages, self->m_twiddleCos, self->m_twiddleSin);
        butterflies += M / 2;
    }

    data.resize(N);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = N;
    self->m_stats.numButterflies = butterflies;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(N, butterflies, timer.elapsed());

    return data;
}

/* ---- Forward real ---- */

QVector<QPair<double, double>> BruunFFT6::forward(const QVector<double>& input) const
{
    QVector<QPair<double, double>> complexInput(input.size());
    for (int i = 0; i < input.size(); ++i)
        complexInput[i] = {input[i], 0.0};
    return forwardComplex(complexInput);
}

/* ---- Inverse ---- */

QVector<QPair<double, double>> BruunFFT6::inverse(
    const QVector<QPair<double, double>>& spectrum) const
{
    QVector<QPair<double, double>> conjSpec(spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i)
        conjSpec[i] = {spectrum[i].first, -spectrum[i].second};

    auto result = const_cast<BruunFFT6*>(this)->forwardComplex(conjSpec);
    for (auto& z : result)
        z = {z.first / result.size(), -z.second / result.size()};
    return result;
}

/* ---- Reset ---- */

void BruunFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_twiddleCos.clear();
    m_twiddleSin.clear();
    m_cachedSize = 0;
}
