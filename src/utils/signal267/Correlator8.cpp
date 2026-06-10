/**
 * @file Correlator8.cpp
 * @brief Correlator8 实现
 *
 * 实现相关器：广义互相关与相位变换加权噪声信道时延估计。
 */

#include "utils/signal267/Correlator8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator8::Correlator8(QObject *parent)
    : QObject(parent) {}

Correlator8::~Correlator8() = default;

/* ---- Configuration ---- */

void Correlator8::setMethod(WeightingMethod method)
{
    m_method = method;
}

/* ---- Utility ---- */

int Correlator8::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- FFT ---- */

void Correlator8::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * re[i+j+len/2] - curIm * im[i+j+len/2];
                double tIm = curRe * im[i+j+len/2] + curIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

void Correlator8::ifft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    fft(re, im);
    for (int i = 0; i < n; ++i) {
        re[i] /= n;
        im[i] = -im[i] / n;
    }
}

/* ---- Power spectrum ---- */

QVector<double> Correlator8::powerSpectrum(const QVector<double>& re,
                                              const QVector<double>& im) const
{
    QVector<double> p(re.size());
    for (int i = 0; i < re.size(); ++i)
        p[i] = re[i] * re[i] + im[i] * im[i];
    return p;
}

/* ---- PHAT weighting ---- */

void Correlator8::applyPHAT(QVector<double>& crossRe,
                              QVector<double>& crossIm) const
{
    for (int i = 0; i < crossRe.size(); ++i) {
        double mag = qSqrt(crossRe[i] * crossRe[i] + crossIm[i] * crossIm[i]);
        if (mag > 1e-10) {
            crossRe[i] /= mag;
            crossIm[i] /= mag;
        } else {
            crossRe[i] = 0.0;
            crossIm[i] = 0.0;
        }
    }
}

/* ---- ML weighting ---- */

void Correlator8::applyML(const QVector<double>& spec1,
                            const QVector<double>& spec2,
                            QVector<double>& crossRe,
                            QVector<double>& crossIm) const
{
    for (int i = 0; i < crossRe.size(); ++i) {
        double snr = (spec1[i] + spec2[i]) / qMax(1e-10, spec1[i] * spec2[i]);
        double w = qMin(snr, 100.0);
        crossRe[i] *= w;
        crossIm[i] *= w;
    }
}

/* ---- GCC ---- */

QVector<double> Correlator8::gcc(const QVector<double>& signal1,
                                    const QVector<double>& signal2,
                                    WeightingMethod method) const
{
    int n1 = signal1.size(), n2 = signal2.size();
    int fftSize = nextPow2(n1 + n2 - 1);

    // Zero-pad both signals
    QVector<double> re1(fftSize, 0.0), im1(fftSize, 0.0);
    QVector<double> re2(fftSize, 0.0), im2(fftSize, 0.0);
    for (int i = 0; i < n1; ++i) re1[i] = signal1[i];
    for (int i = 0; i < n2; ++i) re2[i] = signal2[i];

    // Forward FFT
    fft(re1, im1);
    fft(re2, im2);

    // Cross-spectrum
    QVector<double> crossRe(fftSize), crossIm(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        crossRe[i] = re1[i] * re2[i] + im1[i] * im2[i];  // conj(X1) * X2
        crossIm[i] = im1[i] * re2[i] - re1[i] * im2[i];
    }

    // Apply weighting
    switch (method) {
    case PHAT:
        applyPHAT(crossRe, crossIm);
        break;
    case ML: {
        auto p1 = powerSpectrum(re1, im1);
        auto p2 = powerSpectrum(re2, im2);
        applyML(p1, p2, crossRe, crossIm);
        break;
    }
    case Eckart: {
        auto p1 = powerSpectrum(re1, im1);
        auto p2 = powerSpectrum(re2, im2);
        for (int i = 0; i < fftSize; ++i) {
            double w = 1.0 / qMax(1e-10, qSqrt(p1[i] * p2[i]));
            crossRe[i] *= w;
            crossIm[i] *= w;
        }
        break;
    }
    case SCC: {
        // Smoothed coherence: approximate via spectral averaging
        for (int i = 0; i < fftSize; ++i) {
            double mag = qSqrt(crossRe[i] * crossRe[i] + crossIm[i] * crossIm[i]);
            double g = mag / qMax(1e-10, qSqrt(
                (re1[i]*re1[i]+im1[i]*im1[i]) *
                (re2[i]*re2[i]+im2[i]*im2[i])));
            crossRe[i] *= g;
            crossIm[i] *= g;
        }
        break;
    }
    case Standard:
    default:
        break;
    }

    // Inverse FFT
    ifft(crossRe, crossIm);
    return crossRe;
}

/* ---- Cross-correlate ---- */

QVector<double> Correlator8::correlate(const QVector<double>& signal1,
                                          const QVector<double>& signal2) const
{
    QElapsedTimer timer;
    timer.start();

    auto result = gcc(signal1, signal2, m_method);

    double elapsed = timer.elapsed();
    auto [peak, lag] = findPeak(result);
    m_stats.signalLength = qMax(signal1.size(), signal2.size());
    m_stats.estimatedLag = lag;
    m_stats.peakCorrelation = peak;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit correlationComputed(result.size(), lag, peak, elapsed);
    return result;
}

/* ---- Estimate delay ---- */

int Correlator8::estimateDelay(const QVector<double>& signal1,
                                 const QVector<double>& signal2) const
{
    auto corr = gcc(signal1, signal2, m_method);
    auto [peak, lag] = findPeak(corr);

    // Adjust lag relative to zero-delay center
    int n = corr.size();
    int delay = (lag > n / 2) ? lag - n : lag;

    m_stats.estimatedLag = delay;
    m_stats.peakCorrelation = peak;
    return delay;
}

/* ---- Auto-correlate ---- */

QVector<double> Correlator8::autoCorrelate(const QVector<double>& signal) const
{
    return gcc(signal, signal, Standard);
}

/* ---- Find peak ---- */

QPair<double, int> Correlator8::findPeak(
    const QVector<double>& correlation) const
{
    double peak = -std::numeric_limits<double>::max();
    int lag = 0;
    for (int i = 0; i < correlation.size(); ++i) {
        if (correlation[i] > peak) {
            peak = correlation[i];
            lag = i;
        }
    }
    return {peak, lag};
}

/* ---- Reset ---- */

void Correlator8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
