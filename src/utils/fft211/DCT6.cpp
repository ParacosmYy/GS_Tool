/**
 * @file DCT6.cpp
 * @brief DCT6 实现
 *
 * 实现第二类离散余弦变换：半长度FFT快速计算、偶扩展输入重索引、正交归一化。
 */

#include "utils/fft211/DCT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT6::DCT6(QObject *parent) : QObject(parent) {}
DCT6::~DCT6() = default;

/* ---- Next power of two ---- */

int DCT6::nextPowerOfTwo(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Radix-2 FFT (in-place) ---- */

void DCT6::fftRadix2(QVector<QPair<double, double>>& data)
{
    int N = data.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Butterfly stages
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

/* ---- Prepare even-extended input ---- */

QVector<QPair<double, double>> DCT6::prepareEvenExtend(
    const QVector<double>& input)
{
    int N = input.size();
    // Reindex: y[k] = x[2k] for k < N/2, y[N-1-k] = x[2k+1] for k < N/2
    // Use half-length FFT: form complex sequence from even/odd indexed samples
    int halfN = (N + 1) / 2;
    QVector<QPair<double, double>> z(halfN);

    for (int k = 0; k < halfN; ++k) {
        int evenIdx = 2 * k;
        int oddIdx = 2 * k + 1;
        double evenVal = (evenIdx < N) ? input[evenIdx] : 0.0;
        double oddVal = (oddIdx < N) ? input[oddIdx] : 0.0;
        // Pack into complex: z[k] = even[k] + j*odd[k]
        z[k] = {evenVal, oddVal};
    }

    return z;
}

/* ---- Forward DCT-II ---- */

QVector<double> DCT6::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};

    // Use FFT-based approach:
    // Form Y[k] = FFT of re-indexed sequence
    // DCT[k] = Re{Y[k]} * cos_factor[k]
    int halfN = nextPowerOfTwo((N + 1) / 2);
    QVector<QPair<double, double>> z(halfN, {0.0, 0.0});

    // Reindex input: even-indexed in real, odd-indexed in imaginary
    int half = (N + 1) / 2;
    for (int k = 0; k < half && k < halfN; ++k) {
        int eIdx = 2 * k;
        int oIdx = 2 * k + 1;
        z[k] = {(eIdx < N) ? input[eIdx] : 0.0,
                (oIdx < N) ? input[oIdx] : 0.0};
    }

    fftRadix2(z);

    // Extract DCT coefficients from FFT result
    QVector<double> result(N, 0.0);
    for (int k = 0; k < N; ++k) {
        double angle = M_PI * k / (2.0 * N);
        double cosA = qCos(angle);
        double sinA = qSin(angle);

        // Combine FFT outputs with twiddle factors
        int fk = k % halfN;
        double re = z[fk].first * cosA + z[fk].second * sinA;
        result[k] = 2.0 * re;
    }

    auto self = const_cast<DCT6*>(this);
    self->m_stats.totalTransforms++;
    self->m_stats.lastSize = N;
    self->m_stats.fftSize = halfN;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    self->emit transformCompleted(N, halfN, timer.elapsed());

    return result;
}

/* ---- Inverse DCT-II (Type-III DCT) ---- */

QVector<double> DCT6::inverse(const QVector<double>& spectrum) const
{
    QElapsedTimer timer;
    timer.start();

    int N = spectrum.size();
    if (N == 0) return {};

    // Type-III DCT: x[n] = sum_{k=0}^{N-1} C[k] * cos(pi*(2n+1)*k/(2N))
    // where C[0] has factor 0.5
    QVector<double> result(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double sum = 0.5 * spectrum[0];
        for (int k = 1; k < N; ++k) {
            double angle = M_PI * (2 * n + 1) * k / (2.0 * N);
            sum += spectrum[k] * qCos(angle);
        }
        result[n] = sum / N * 2.0;
    }

    auto self = const_cast<DCT6*>(this);
    self->m_stats.totalTransforms++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return result;
}

/* ---- Orthonormal forward ---- */

QVector<double> DCT6::forwardOrtho(const QVector<double>& input) const
{
    auto result = forward(input);
    int N = result.size();
    double scale = qSqrt(2.0 / N);
    for (int k = 0; k < N; ++k) {
        double s = (k == 0) ? qSqrt(0.5) : 1.0;
        result[k] = result[k] * scale * s;
    }
    return result;
}

/* ---- Orthonormal inverse ---- */

QVector<double> DCT6::inverseOrtho(const QVector<double>& spectrum) const
{
    auto result = inverse(spectrum);
    int N = result.size();
    double scale = qSqrt(2.0 / N);
    for (int n = 0; n < N; ++n)
        result[n] *= scale;
    return result;
}

/* ---- Naive DCT-II ---- */

QVector<double> DCT6::naiveDCTII(const QVector<double>& input)
{
    int N = input.size();
    QVector<double> result(N, 0.0);
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += input[n] * qCos(M_PI * (2 * n + 1) * k / (2.0 * N));
        }
        result[k] = 2.0 * sum;
    }
    return result;
}

/* ---- Reset ---- */

void DCT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
