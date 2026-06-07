/**
 * @file ChirpZ5.cpp
 * @brief ChirpZ5 实现
 *
 * 实现Chirp-Z变换：Bluestein算法、自动分辨率选择、频率导数分析。
 */

#include "utils/fft194/ChirpZ5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChirpZ5::ChirpZ5(QObject *parent) : QObject(parent) {}
ChirpZ5::~ChirpZ5() = default;

/* ---- Configuration ---- */

void ChirpZ5::setFreqRange(double fmin, double fmax)
{
    m_fmin = qBound(0.0, fmin, 0.5);
    m_fmax = qBound(m_fmin + 1e-6, fmax, 0.5);
}

void ChirpZ5::setOutputSize(int m)
{
    m_outputSize = qMax(0, m);  // 0 = auto
}

/* ---- Next power of 2 ---- */

int ChirpZ5::nextPow2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Auto-select resolution ---- */

int ChirpZ5::autoResolution(int inputLen) const
{
    // At least inputLen points, prefer next pow2
    if (inputLen <= 0) return 64;
    int res = qMax(inputLen, 64);
    return nextPow2(res);
}

/* ---- Generate chirp twiddle factors ---- */

void ChirpZ5::generateTwiddles(int N, int M,
                                QVector<QVector<double>>& wk,
                                QVector<QVector<double>>& aExp) const
{
    // Contour: z_k = A * W^{-k}, k = 0..M-1
    // A = exp(j*2*pi*fmin), W = exp(-j*2*pi*(fmax-fmin)/M)
    double phiA = 2.0 * M_PI * m_fmin;
    double phiW = -2.0 * M_PI * (m_fmax - m_fmin) / M;

    wk.resize(M);
    aExp.resize(N);

    for (int k = 0; k < M; ++k) {
        double angle = phiW * k;
        wk[k] = {qCos(angle), qSin(angle)};
    }
    for (int n = 0; n < N; ++n) {
        double angle = phiA * n;
        aExp[n] = {qCos(angle), qSin(angle)};
    }
}

/* ---- Bluestein CZT via convolution ---- */

QVector<QVector<double>> ChirpZ5::bluesteinCZT(
    const QVector<double>& x, int M,
    const QVector<QVector<double>>& wk) const
{
    int N = x.size();
    int L = nextPow2(N + M - 1);  // convolution length

    // Build chirp: y[n] = x[n] * A^{-n} * W^{n^2/2}
    QVector<QVector<double>> y(L, {0.0, 0.0});
    QVector<QVector<double>> chirp(L, {0.0, 0.0});

    for (int n = 0; n < N; ++n) {
        int n2 = n * n;
        // W^{n^2/2} factor
        double angle = -M_PI * (m_fmax - m_fmin) * n2 / M;
        double wr = qCos(angle), wi = qSin(angle);
        // A^{-n} factor
        double ar = qCos(-2.0 * M_PI * m_fmin * n);
        double ai = qSin(-2.0 * M_PI * m_fmin * n);
        // Multiply: (ar+j*ai)*(wr+j*wi)
        double r = ar * wr - ai * wi;
        double im = ar * wi + ai * wr;
        y[n] = {x[n] * r, x[n] * im};
    }

    // Chirp filter: h[n] = W^{n^2/2} for |n| < L
    for (int n = 0; n < L; ++n) {
        int idx = (n <= L / 2) ? n : n - L;
        int idx2 = idx * idx;
        double angle = -M_PI * (m_fmax - m_fmin) * idx2 / M;
        chirp[n] = {qCos(angle), qSin(angle)};
    }

    // Simplified circular convolution (direct O(N*M) for correctness)
    QVector<QVector<double>> result(M, {0.0, 0.0});
    for (int k = 0; k < M; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            int ci = ((k - n) % L + L) % L;
            // Multiply y[n] * chirp[ci]
            sr += y[n][0] * chirp[ci][0] - y[n][1] * chirp[ci][1];
            si += y[n][0] * chirp[ci][1] + y[n][1] * chirp[ci][0];
        }
        // Apply final W^{k^2/2} * A^k
        int k2 = k * k;
        double angle = M_PI * (m_fmax - m_fmin) * k2 / M;
        double fr = qCos(angle), fi = qSin(angle);
        result[k] = {sr * fr - si * fi, sr * fi + si * fr};
    }

    return result;
}

/* ---- Compute CZT ---- */

QVector<QVector<double>> ChirpZ5::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};

    int M = (m_outputSize > 0) ? m_outputSize : autoResolution(N);
    M = qMax(1, M);

    QVector<QVector<double>> wk, aExp;
    generateTwiddles(N, M, wk, aExp);

    QVector<QVector<double>> result = bluesteinCZT(input, M, wk);

    // Compute frequency bins
    m_freqBins.resize(M);
    double step = (m_fmax - m_fmin) / M;
    for (int k = 0; k < M; ++k)
        m_freqBins[k] = m_fmin + k * step;

    m_stats.totalTransforms++;
    m_stats.lastInputSize = N;
    m_stats.lastOutputSize = M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M, timer.elapsed());
    return result;
}

/* ---- Frequency derivative ---- */

QVector<double> ChirpZ5::frequencyDerivative(
    const QVector<QVector<double>>& spectrum) const
{
    int M = spectrum.size();
    if (M < 2) return {};

    QVector<double> deriv(M - 1, 0.0);
    for (int k = 0; k < M - 1; ++k) {
        double mag1 = qSqrt(spectrum[k][0] * spectrum[k][0] +
                            spectrum[k][1] * spectrum[k][1]);
        double mag2 = qSqrt(spectrum[k + 1][0] * spectrum[k + 1][0] +
                            spectrum[k + 1][1] * spectrum[k + 1][1]);
        double df = (m_freqBins.size() > k + 1)
                    ? m_freqBins[k + 1] - m_freqBins[k]
                    : 1.0;
        deriv[k] = (df > 1e-15) ? (mag2 - mag1) / df : 0.0;
    }
    return deriv;
}

/* ---- Frequency bins ---- */

QVector<double> ChirpZ5::frequencyBins() const
{
    return m_freqBins;
}

/* ---- Reset ---- */

void ChirpZ5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_freqBins.clear();
}
