/**
 * @file ChirpZ7.cpp
 * @brief ChirpZ7 实现
 *
 * 实现Chirp Z变换：Bluestein递归、任意轮廓采样、Zoom-FFT。
 */

#include "utils/fft207/ChirpZ7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChirpZ7::ChirpZ7(QObject *parent) : QObject(parent) {}
ChirpZ7::~ChirpZ7() = default;

/* ---- Configuration ---- */

void ChirpZ7::setContour(const QPair<double, double>& A, const QPair<double, double>& W)
{
    m_Ar = A.first;  m_Ai = A.second;
    m_Wr = W.first;  m_Wi = W.second;
}

void ChirpZ7::setOutputSize(int M) { m_M = qMax(1, M); }

/* ---- Complex helpers ---- */

QPair<double, double> ChirpZ7::cmul(const QPair<double, double>& a,
                                      const QPair<double, double>& b)
{
    return {a.first * b.first - a.second * b.second,
            a.first * b.second + a.second * b.first};
}

QPair<double, double> ChirpZ7::conj(const QPair<double, double>& z)
{
    return {z.first, -z.second};
}

/* ---- Bit reverse ---- */

int ChirpZ7::bitReverse(int x, int bits)
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- Next power of 2 ---- */

int ChirpZ7::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- In-place radix-2 FFT ---- */

void ChirpZ7::fftImpl(QVector<QPair<double, double>>& data, bool inverse)
{
    int N = data.size();
    if (N <= 1) return;

    int bits = 0;
    while ((1 << bits) < N) ++bits;

    // Bit-reversal permutation
    for (int i = 0; i < N; ++i) {
        int j = bitReverse(i, bits);
        if (j > i) std::swap(data[i], data[j]);
    }

    // Butterfly stages
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
        for (auto& z : data) {
            z.first /= N;
            z.second /= N;
        }
    }
}

/* ---- CZT via Bluestein ---- */

QVector<QPair<double, double>> ChirpZ7::transformComplex(
    const QVector<QPair<double, double>>& input) const
{
    QElapsedTimer timer;
    timer.start();
    int N = input.size();
    int M = m_M;
    int L = nextPow2(N + M - 1);

    // Build chirp sequences
    QVector<QPair<double, double>> y(L, {0.0, 0.0});
    QVector<QPair<double, double>> v(L, {0.0, 0.0});

    for (int n = 0; n < N; ++n) {
        // y[n] = x[n] * A^{-n} * W^{n^2/2}
        double angA = -n * qAtan2(m_Ai, m_Ar);
        double magA = qSqrt(m_Ar * m_Ar + m_Ai * m_Ai);
        double angW = n * n * 0.5 * qAtan2(m_Wi, m_Wr);
        double magW = qPow(qSqrt(m_Wr * m_Wr + m_Wi * m_Wi), n * n * 0.5);
        double cmag = qPow(magA, -n) * magW;
        double cang = angA + angW;
        y[n] = cmul(input[n], {cmag * qCos(cang), cmag * qSin(cang)});
    }

    // v[k] = W^{k^2/2} for k in [-(N-1), M-1], wrapped
    for (int k = 0; k < M + N - 1; ++k) {
        int idx = (k <= (N - 1)) ? k : L - (N - 1 - k + N - 1);
        idx = idx % L;
        double angW = -k * k * 0.5 * qAtan2(m_Wi, m_Wr);
        double magW = qPow(qSqrt(m_Wr * m_Wr + m_Wi * m_Wi), -k * k * 0.5);
        v[idx].first += magW * qCos(angW);
        v[idx].second += magW * qSin(angW);
    }

    // Convolution via FFT: Y * V
    fftImpl(y, false);
    fftImpl(v, false);
    for (int i = 0; i < L; ++i)
        y[i] = cmul(y[i], v[i]);
    fftImpl(y, true);

    // Extract output and multiply by W^{k^2/2}
    QVector<QPair<double, double>> output(M);
    for (int k = 0; k < M; ++k) {
        double angW = k * k * 0.5 * qAtan2(m_Wi, m_Wr);
        double magW = qPow(qSqrt(m_Wr * m_Wr + m_Wi * m_Wi), k * k * 0.5);
        output[k] = cmul(y[k], {magW * qCos(angW), magW * qSin(angW)});
    }

    const_cast<ChirpZ7*>(this)->m_stats.totalTransforms++;
    const_cast<ChirpZ7*>(this)->m_stats.inputSize = N;
    const_cast<ChirpZ7*>(this)->m_stats.outputSize = M;
    const_cast<ChirpZ7*>(this)->m_timeSum += timer.elapsed();
    const_cast<ChirpZ7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalTransforms;
    const_cast<ChirpZ7*>(this)->emit transformCompleted(N, M, timer.elapsed());

    return output;
}

/* ---- Real-valued CZT ---- */

QVector<QPair<double, double>> ChirpZ7::transform(const QVector<double>& input) const
{
    QVector<QPair<double, double>> complexInput(input.size());
    for (int i = 0; i < input.size(); ++i)
        complexInput[i] = {input[i], 0.0};
    return transformComplex(complexInput);
}

/* ---- Zoom FFT on unit circle ---- */

QVector<QPair<double, double>> ChirpZ7::zoomFFT(const QVector<double>& input,
                                                   double fLow, double fHigh) const
{
    int N = input.size();
    // Set contour on unit circle from fLow to fHigh
    double wAngle = -2.0 * M_PI * (fHigh - fLow) / (m_M * N);
    const_cast<ChirpZ7*>(this)->setContour(
        {qCos(2.0 * M_PI * fLow / N), qSin(2.0 * M_PI * fLow / N)},
        {qCos(wAngle), qSin(wAngle)});
    return transform(input);
}

/* ---- Inverse CZT ---- */

QVector<QPair<double, double>> ChirpZ7::inverseTransform(
    const QVector<QPair<double, double>>& spectrum, int N) const
{
    // Inverse CZT: swap roles of A and W^-1
    double invWr = m_Wr, invWi = -m_Wi;
    ChirpZ7 inv;
    inv.setContour({m_Ar, -m_Ai}, {invWr, invWi});
    inv.setOutputSize(N);
    return inv.transformComplex(spectrum);
}

/* ---- Reset ---- */

void ChirpZ7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
