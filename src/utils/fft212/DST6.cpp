/**
 * @file DST6.cpp
 * @brief DST6 实现
 *
 * 实现第二类离散正弦变换：奇对称扩展、DCT快速计算流水线、正交归一化。
 */

#include "utils/fft212/DST6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST6::DST6(QObject *parent) : QObject(parent) {}
DST6::~DST6() = default;

/* ---- Next power of two ---- */

int DST6::nextPowerOfTwo(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Radix-2 FFT ---- */

void DST6::fftRadix2(QVector<QPair<double, double>>& data)
{
    int N = data.size();
    if (N <= 1) return;

    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        QPair<double, double> wN = {qCos(angle), qSin(angle)};
        for (int i = 0; i < N; i += len) {
            QPair<double, double> w = {1.0, 0.0};
            for (int j = 0; j < len / 2; ++j) {
                auto& even = data[i + j];
                auto& odd = data[i + j + len / 2];
                double re = w.first * odd.first - w.second * odd.second;
                double im = w.first * odd.second + w.second * odd.first;
                odd = {even.first - re, even.second - im};
                even = {even.first + re, even.second + im};
                double wr = w.first * wN.first - w.second * wN.second;
                double wi = w.first * wN.second + w.second * wN.first;
                w = {wr, wi};
            }
        }
    }
}

/* ---- Build odd-symmetric extension ---- */

QVector<double> DST6::buildOddSymmetric(const QVector<double>& input)
{
    // DST-II via DCT: extend x[n] with odd symmetry
    // y[n] = x[n] for n < N, y[2N-1-n] = -x[n] for n < N
    int N = input.size();
    QVector<double> extended(2 * N, 0.0);
    for (int n = 0; n < N; ++n) {
        extended[n] = input[n];
        extended[2 * N - 1 - n] = -input[n];
    }
    return extended;
}

/* ---- Forward DST-II ---- */

QVector<double> DST6::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};

    // DST-II: S[k] = sum_{n=0}^{N-1} x[n] * sin(pi*(2n+1)*(k+1)/(2N))
    // Compute via FFT of odd-symmetric extended sequence
    int extN = nextPowerOfTwo(2 * N);
    QVector<QPair<double, double>> fftData(extN, {0.0, 0.0});

    // Fill with odd-symmetric extension
    for (int n = 0; n < N; ++n) {
        fftData[n] = {input[n], 0.0};
        int mirrorIdx = 2 * N - 1 - n;
        if (mirrorIdx < extN)
            fftData[mirrorIdx] = {-input[n], 0.0};
    }

    fftRadix2(fftData);

    // Extract DST coefficients: S[k] = Im{Y[k+1]}
    QVector<double> result(N, 0.0);
    for (int k = 0; k < N; ++k) {
        int fk = k + 1;
        if (fk < extN) {
            result[k] = -fftData[fk].second;  // Imaginary part with sign
        }
    }

    auto self = const_cast<DST6*>(this);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = N;
    self->m_stats.fftSize = extN;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(N, extN, timer.elapsed());

    return result;
}

/* ---- Inverse DST-II (DST-III) ---- */

QVector<double> DST6::inverse(const QVector<double>& spectrum) const
{
    QElapsedTimer timer;
    timer.start();

    int N = spectrum.size();
    if (N == 0) return {};

    // DST-III: x[n] = (2/N) * sum_{k=0}^{N-1} S[k] * sin(pi*(2n+1)*(k+1)/(2N))
    // where S[N-1] has factor 0.5
    QVector<double> result(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double coeff = (k == N - 1) ? 0.5 : 1.0;
            sum += coeff * spectrum[k] * qSin(M_PI * (2 * n + 1) * (k + 1) / (2.0 * N));
        }
        result[n] = 2.0 * sum / N;
    }

    auto self = const_cast<DST6*>(this);
    self->m_stats.totalTransforms++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return result;
}

/* ---- Orthonormal forward ---- */

QVector<double> DST6::forwardOrtho(const QVector<double>& input) const
{
    auto result = forward(input);
    int N = result.size();
    double scale = qSqrt(2.0 / N);
    for (int k = 0; k < N; ++k)
        result[k] *= scale;
    return result;
}

/* ---- Orthonormal inverse ---- */

QVector<double> DST6::inverseOrtho(const QVector<double>& spectrum) const
{
    auto result = inverse(spectrum);
    int N = result.size();
    double scale = qSqrt(2.0 / N);
    for (int n = 0; n < N; ++n)
        result[n] *= scale;
    return result;
}

/* ---- Naive DST-II ---- */

QVector<double> DST6::naiveDSTII(const QVector<double>& input)
{
    int N = input.size();
    QVector<double> result(N, 0.0);
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n)
            sum += input[n] * qSin(M_PI * (2 * n + 1) * (k + 1) / (2.0 * N));
        result[k] = sum;
    }
    return result;
}

/* ---- Reset ---- */

void DST6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
