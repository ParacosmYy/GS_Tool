/**
 * @file Correlator2.cpp
 * @brief Correlator2 实现
 *
 * 实现广义互相关GCC-PHAT：时延估计、多种加权、子采样峰值。
 */

#include "utils/signal197/Correlator2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator2::Correlator2(QObject *parent) : QObject(parent) {}
Correlator2::~Correlator2() = default;

/* ---- Configuration ---- */

void Correlator2::setWeighting(Weighting w) { m_weighting = w; }
void Correlator2::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

/* ---- Next power of 2 ---- */

int Correlator2::nextPow2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- DFT (direct O(n^2)) ---- */

QVector<QVector<double>> Correlator2::dft(const QVector<double>& x) const
{
    int N = x.size();
    QVector<QVector<double>> X(N, {0.0, 0.0});
    for (int k = 0; k < N; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            sr += x[n] * qCos(angle);
            si += x[n] * qSin(angle);
        }
        X[k] = {sr, si};
    }
    return X;
}

/* ---- Inverse DFT ---- */

QVector<double> Correlator2::idft(const QVector<QVector<double>>& X) const
{
    int N = X.size();
    QVector<double> x(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double sr = 0.0;
        for (int k = 0; k < N; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            sr += X[k][0] * qCos(angle) - X[k][1] * qSin(angle);
        }
        x[n] = sr / N;
    }
    return x;
}

/* ---- Apply weighting ---- */

QVector<QVector<double>> Correlator2::applyWeighting(
    const QVector<QVector<double>>& Gxy) const
{
    int N = Gxy.size();
    QVector<QVector<double>> W(N, {0.0, 0.0});

    switch (m_weighting) {
    case PHAT:
        // Phase Transform: W = Gxy / |Gxy|
        for (int k = 0; k < N; ++k) {
            double mag = qSqrt(Gxy[k][0] * Gxy[k][0] + Gxy[k][1] * Gxy[k][1]);
            if (mag > 1e-10) { W[k] = {Gxy[k][0] / mag, Gxy[k][1] / mag}; }
        }
        break;
    case SCOT:
        // Smoothed Coherence Transform
        for (int k = 0; k < N; ++k) {
            double mag = qSqrt(Gxy[k][0] * Gxy[k][0] + Gxy[k][1] * Gxy[k][1]);
            double smoothed = mag + 1e-10;
            W[k] = {Gxy[k][0] / smoothed, Gxy[k][1] / smoothed};
        }
        break;
    case ML:
        // Maximum Likelihood weighting
        for (int k = 0; k < N; ++k) {
            double mag2 = Gxy[k][0] * Gxy[k][0] + Gxy[k][1] * Gxy[k][1];
            double denom = 1.0 - mag2 / (mag2 + 1.0);
            if (qAbs(denom) > 1e-10)
                W[k] = {Gxy[k][0] / denom, Gxy[k][1] / denom};
        }
        break;
    case Eckart:
        // Eckart filter: |Gxy| / (Gxx * Gyy)
        for (int k = 0; k < N; ++k) {
            double mag = qSqrt(Gxy[k][0] * Gxy[k][0] + Gxy[k][1] * Gxy[k][1]);
            W[k] = {Gxy[k][0] * mag, Gxy[k][1] * mag};
        }
        break;
    case None:
    default:
        W = Gxy;
        break;
    }
    return W;
}

/* ---- Cross-correlation ---- */

QVector<double> Correlator2::correlate(const QVector<double>& x,
                                        const QVector<double>& y) const
{
    QElapsedTimer timer;
    timer.start();

    int nx = x.size(), ny = y.size();
    int N = nextPow2(nx + ny - 1);
    if (N == 0) return {};

    // Zero-pad to length N
    QVector<double> xp(N, 0.0), yp(N, 0.0);
    for (int i = 0; i < nx; ++i) xp[i] = x[i];
    for (int i = 0; i < ny; ++i) yp[i] = y[i];

    // Compute DFTs
    auto Xf = dft(xp);
    auto Yf = dft(yp);

    // Cross-spectrum: Gxy = X * conj(Y)
    QVector<QVector<double>> Gxy(N, {0.0, 0.0});
    for (int k = 0; k < N; ++k) {
        Gxy[k] = {Xf[k][0] * Yf[k][0] + Xf[k][1] * Yf[k][1],
                   Xf[k][1] * Yf[k][0] - Xf[k][0] * Yf[k][1]};
    }

    // Apply weighting
    auto W = applyWeighting(Gxy);

    // Inverse DFT
    QVector<double> corr = idft(W);

    // Return valid portion
    int resultLen = nx + ny - 1;
    QVector<double> result(resultLen, 0.0);
    for (int i = 0; i < resultLen; ++i) result[i] = corr[i];

    const_cast<Correlator2*>(this)->m_stats.totalCorrelations++;
    const_cast<Correlator2*>(this)->m_stats.lastLength = N;
    m_timeSum += timer.elapsed();
    const_cast<Correlator2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalCorrelations;

    return result;
}

/* ---- Find peak with parabolic interpolation ---- */

QPair<double, double> Correlator2::findPeak(const QVector<double>& corr) const
{
    if (corr.isEmpty()) return {0.0, 0.0};

    int maxIdx = 0;
    double maxVal = corr[0];
    for (int i = 1; i < corr.size(); ++i) {
        if (corr[i] > maxVal) { maxVal = corr[i]; maxIdx = i; }
    }

    // Parabolic interpolation for sub-sample precision
    double refinedIdx = maxIdx;
    double refinedVal = maxVal;

    if (maxIdx > 0 && maxIdx < corr.size() - 1) {
        double y0 = corr[maxIdx - 1];
        double y1 = corr[maxIdx];
        double y2 = corr[maxIdx + 1];
        double denom = 2.0 * (2.0 * y1 - y0 - y2);
        if (qAbs(denom) > 1e-15) {
            double delta = (y0 - y2) / denom;
            delta = qBound(-0.5, delta, 0.5);
            refinedIdx = maxIdx + delta;
            refinedVal = y1 - 0.25 * (y0 - y2) * delta;
        }
    }

    return {refinedIdx, refinedVal};
}

/* ---- Estimate delay ---- */

double Correlator2::estimateDelay(const QVector<double>& x,
                                   const QVector<double>& y) const
{
    QElapsedTimer timer;
    timer.start();

    auto corr = correlate(x, y);
    auto peak = findPeak(corr);

    // Convert peak index to delay (relative to center)
    int centerLag = x.size() - 1;
    double delay = peak.first - centerLag;

    const_cast<Correlator2*>(this)->m_stats.lastDelay = delay;
    const_cast<Correlator2*>(this)->m_stats.lastPeakValue = peak.second;

    const_cast<Correlator2*>(this)->correlationCompleted(
        x.size(), delay, peak.second, timer.elapsed());

    return delay;
}

/* ---- Estimate delay in seconds ---- */

double Correlator2::estimateDelaySeconds(const QVector<double>& x,
                                          const QVector<double>& y) const
{
    double delaySamples = estimateDelay(x, y);
    return delaySamples / m_sampleRate;
}

/* ---- Auto-correlation ---- */

QVector<double> Correlator2::autoCorrelate(const QVector<double>& x) const
{
    return correlate(x, x);
}

/* ---- Reset ---- */

void Correlator2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
