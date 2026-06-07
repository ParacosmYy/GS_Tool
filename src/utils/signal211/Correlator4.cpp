/**
 * @file Correlator4.cpp
 * @brief Correlator4 实现
 *
 * 实现互相关器：GCC-PHAT、子采样抛物线插值、时延估计。
 */

#include "utils/signal211/Correlator4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator4::Correlator4(QObject *parent) : QObject(parent) {}
Correlator4::~Correlator4() = default;

/* ---- Configuration ---- */

void Correlator4::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void Correlator4::setWeighting(GccWeighting w) { m_weighting = w; }

/* ---- Next power of 2 ---- */

int Correlator4::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Naive DFT ---- */

void Correlator4::dft(QVector<QPair<double, double>>& data, bool inverse)
{
    int N = data.size();
    QVector<QPair<double, double>> out(N, {0.0, 0.0});
    double sign = inverse ? 1.0 : -1.0;

    for (int k = 0; k < N; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / N;
            re += data[n].first * qCos(angle) - data[n].second * qSin(angle);
            im += data[n].first * qSin(angle) + data[n].second * qCos(angle);
        }
        if (inverse) { re /= N; im /= N; }
        out[k] = {re, im};
    }
    data = out;
}

/* ---- Magnitude ---- */

QVector<double> Correlator4::magnitude(const QVector<QPair<double, double>>& spec)
{
    QVector<double> mag(spec.size());
    for (int i = 0; i < spec.size(); ++i)
        mag[i] = qSqrt(spec[i].first * spec[i].first + spec[i].second * spec[i].second);
    return mag;
}

/* ---- Apply GCC weighting ---- */

void Correlator4::applyWeighting(QVector<QPair<double, double>>& crossSpec,
                                   const QVector<QPair<double, double>>& specX,
                                   const QVector<QPair<double, double>>& specY) const
{
    int N = crossSpec.size();

    switch (m_weighting) {
    case None:
        break;

    case Phat: {
        // GCC-PHAT: weight by 1/|cross-spectrum|
        for (int i = 0; i < N; ++i) {
            double mag = qSqrt(crossSpec[i].first * crossSpec[i].first
                               + crossSpec[i].second * crossSpec[i].second);
            if (mag > 1e-10) {
                crossSpec[i].first /= mag;
                crossSpec[i].second /= mag;
            }
        }
        break;
    }

    case Scot: {
        // SCOT: weight by 1/sqrt(|X|^2 * |Y|^2)
        for (int i = 0; i < N; ++i) {
            double magX = specX[i].first * specX[i].first
                          + specX[i].second * specX[i].second;
            double magY = specY[i].first * specY[i].first
                          + specY[i].second * specY[i].second;
            double denom = qSqrt(qMax(magX * magY, 1e-20));
            crossSpec[i].first /= denom;
            crossSpec[i].second /= denom;
        }
        break;
    }

    case Ml: {
        // ML: maximum likelihood weighting
        for (int i = 0; i < N; ++i) {
            double magX = specX[i].first * specX[i].first
                          + specX[i].second * specX[i].second;
            double magY = specY[i].first * specY[i].first
                          + specY[i].second * specY[i].second;
            double gamma = crossSpec[i].first * crossSpec[i].first
                           + crossSpec[i].second * crossSpec[i].second;
            double denom = qMax(magX * magY - gamma, 1e-20);
            double weight = gamma / denom;
            crossSpec[i].first *= weight;
            crossSpec[i].second *= weight;
        }
        break;
    }
    }
}

/* ---- Cross-correlation ---- */

QVector<double> Correlator4::correlate(const QVector<double>& x,
                                         const QVector<double>& y) const
{
    int nx = x.size(), ny = y.size();
    int len = qMax(nx, ny);
    int resultLen = 2 * len - 1;
    QVector<double> result(resultLen, 0.0);

    // Direct time-domain cross-correlation
    for (int lag = -(len - 1); lag < len; ++lag) {
        double sum = 0.0;
        for (int n = 0; n < nx; ++n) {
            int m = n - lag;
            if (m >= 0 && m < ny)
                sum += x[n] * y[m];
        }
        result[lag + len - 1] = sum;
    }
    return result;
}

/* ---- GCC ---- */

QVector<QPair<int, double>> Correlator4::gcc(const QVector<double>& x,
                                                const QVector<double>& y) const
{
    QElapsedTimer timer;
    timer.start();

    int N = nextPow2(qMax(x.size(), y.size()) * 2);

    // Zero-pad and transform both signals
    QVector<QPair<double, double>> specX(N, {0.0, 0.0});
    QVector<QPair<double, double>> specY(N, {0.0, 0.0});
    for (int i = 0; i < x.size(); ++i) specX[i] = {x[i], 0.0};
    for (int i = 0; i < y.size(); ++i) specY[i] = {y[i], 0.0};

    dft(specX, false);
    dft(specY, false);

    // Cross-spectrum: X * conj(Y)
    QVector<QPair<double, double>> crossSpec(N);
    for (int i = 0; i < N; ++i) {
        crossSpec[i] = {
            specX[i].first * specY[i].first + specX[i].second * specY[i].second,
            specX[i].second * specY[i].first - specX[i].first * specY[i].second
        };
    }

    applyWeighting(crossSpec, specX, specY);

    // Inverse DFT to get GCC
    dft(crossSpec, true);

    // Build result with lag indices
    int halfN = N / 2;
    QVector<QPair<int, double>> result(N);
    for (int i = 0; i < N; ++i) {
        int lag = (i < halfN) ? i : i - N;
        result[i] = {lag, crossSpec[i].first};
    }

    auto self = const_cast<Correlator4*>(this);
    self->m_stats.totalOps++;
    self->m_stats.lastSignalLen = qMax(x.size(), y.size());
    self->m_stats.lastFftSize = N;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Sub-sample parabolic interpolation ---- */

double Correlator4::findSubsamplePeak(const QVector<double>& corr, int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= corr.size() - 1) return static_cast<double>(peakIdx);

    double y0 = corr[peakIdx - 1];
    double y1 = corr[peakIdx];
    double y2 = corr[peakIdx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-30) return static_cast<double>(peakIdx);

    // Parabolic interpolation offset
    double delta = (y0 - y2) / denom;
    return static_cast<double>(peakIdx) + delta;
}

/* ---- Estimate delay ---- */

double Correlator4::estimateDelay(const QVector<double>& x,
                                    const QVector<double>& y) const
{
    QElapsedTimer timer;
    timer.start();

    auto gccResult = gcc(x, y);

    // Find peak in GCC
    double maxVal = -std::numeric_limits<double>::max();
    int peakLag = 0;
    for (const auto& [lag, val] : gccResult) {
        if (val > maxVal) {
            maxVal = val;
            peakLag = lag;
        }
    }

    // Extract correlation values for sub-sample interpolation
    QVector<double> corrVals(gccResult.size());
    for (int i = 0; i < gccResult.size(); ++i)
        corrVals[i] = gccResult[i].second;

    int peakIdx = 0;
    for (int i = 0; i < gccResult.size(); ++i) {
        if (gccResult[i].first == peakLag) { peakIdx = i; break; }
    }

    double subsamplePeak = findSubsamplePeak(corrVals, peakIdx);
    double delaySamples = subsamplePeak - static_cast<double>(gccResult.size() / 2);
    double delayMs = delaySamples / m_sampleRate * 1000.0;

    auto self = const_cast<Correlator4*>(this);
    self->m_stats.lastPeakLag = peakLag;
    self->m_stats.lastPeakDelay = delayMs;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit correlationCompleted(
        qMax(x.size(), y.size()), peakLag, delayMs, timer.elapsed());

    return delayMs;
}

/* ---- Autocorrelation ---- */

QVector<double> Correlator4::autocorrelate(const QVector<double>& x) const
{
    return correlate(x, x);
}

/* ---- Reset ---- */

void Correlator4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
