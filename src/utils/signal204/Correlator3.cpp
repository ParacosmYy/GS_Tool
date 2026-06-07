/**
 * @file Correlator3.cpp
 * @brief Correlator3 实现
 *
 * 实现GCC-PHAT互相关：广义互相关、相干性门控、时延估计。
 */

#include "utils/signal204/Correlator3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator3::Correlator3(QObject *parent) : QObject(parent) {}
Correlator3::~Correlator3() = default;

/* ---- Configuration ---- */

void Correlator3::setFrameSize(int size) { m_frameSize = qMax(16, size); }
void Correlator3::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void Correlator3::setCoherenceThreshold(double threshold) { m_coherenceThreshold = qBound(0.0, threshold, 1.0); }

/* ---- Radix-2 FFT ---- */

void Correlator3::fft(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    int n = real.size();
    if (n <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = curRe * real[v] - curIm * imag[v];
                double tIm = curRe * imag[v] + curIm * real[v];
                real[v] = real[u] - tRe;
                imag[v] = imag[u] - tIm;
                real[u] += tRe;
                imag[u] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
    if (inverse) {
        for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] /= n; }
    }
}

/* ---- Auto-power spectrum ---- */

QVector<double> Correlator3::autoPower(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<double> re = signal, im(n, 0.0);
    fft(re, im, false);
    QVector<double> power(n);
    for (int k = 0; k < n; ++k)
        power[k] = re[k] * re[k] + im[k] * im[k];
    return power;
}

/* ---- Cross-power spectrum ---- */

QVector<double> Correlator3::crossPowerSpectrum(const QVector<double>& xReal,
                                                     const QVector<double>& xImag,
                                                     const QVector<double>& yReal,
                                                     const QVector<double>& yImag) const
{
    int n = qMin(qMin(xReal.size(), xImag.size()), qMin(yReal.size(), yImag.size()));
    QVector<double> cps(n);
    for (int k = 0; k < n; ++k)
        cps[k] = xReal[k] * yReal[k] + xImag[k] * yImag[k]; // real part of X * conj(Y)
    return cps;
}

/* ---- Phase transform ---- */

QVector<double> Correlator3::phaseTransform(const QVector<double>& cpsReal,
                                                 const QVector<double>& cpsImag) const
{
    int n = qMin(cpsReal.size(), cpsImag.size());
    QVector<double> weight(n);
    for (int k = 0; k < n; ++k) {
        double mag = qSqrt(cpsReal[k] * cpsReal[k] + cpsImag[k] * cpsImag[k]);
        weight[k] = (mag > 1e-15) ? 1.0 / mag : 0.0;
    }
    return weight;
}

/* ---- Coherence ---- */

QVector<double> Correlator3::coherence(const QVector<double>& x,
                                            const QVector<double>& y) const
{
    int n = qMin(x.size(), y.size());
    QVector<double> autoX = autoPower(x);
    QVector<double> autoY = autoPower(y);

    // Cross spectrum
    QVector<double> xRe = x, xIm(n, 0.0);
    QVector<double> yRe = y, yIm(n, 0.0);
    fft(xRe, xIm, false);
    fft(yRe, yIm, false);

    QVector<double> coh(n);
    for (int k = 0; k < n; ++k) {
        double crossRe = xRe[k] * yRe[k] + xIm[k] * yIm[k];
        double crossIm = xIm[k] * yRe[k] - xRe[k] * yIm[k];
        double crossMag2 = crossRe * crossRe + crossIm * crossIm;
        double denom = autoX[k] * autoY[k];
        coh[k] = (denom > 1e-30) ? crossMag2 / denom : 0.0;
    }
    return coh;
}

/* ---- Apply coherence gate ---- */

void Correlator3::applyCoherenceGate(QVector<double>& gcc, const QVector<double>& coh) const
{
    int n = qMin(gcc.size(), coh.size());
    for (int k = 0; k < n; ++k)
        if (coh[k] < m_coherenceThreshold) gcc[k] = 0.0;
}

/* ---- Find peak with parabolic interpolation ---- */

double Correlator3::findPeak(const QVector<double>& corr) const
{
    int n = corr.size();
    if (n == 0) return 0.0;

    int peakIdx = 0;
    double peakVal = corr[0];
    for (int i = 1; i < n; ++i)
        if (corr[i] > peakVal) { peakVal = corr[i]; peakIdx = i; }

    // Parabolic interpolation for sub-sample precision
    if (peakIdx > 0 && peakIdx < n - 1) {
        double y0 = corr[peakIdx - 1], y1 = corr[peakIdx], y2 = corr[peakIdx + 1];
        double delta = 0.5 * (y0 - y2) / (y0 - 2.0 * y1 + y2 + 1e-30);
        delta = qBound(-0.5, delta, 0.5);
        return peakIdx + delta;
    }
    return static_cast<double>(peakIdx);
}

/* ---- GCC-PHAT ---- */

double Correlator3::gccPhat(const QVector<double>& x, const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMax(x.size(), y.size());
    // Pad to next power of 2
    int fftSize = 1;
    while (fftSize < n) fftSize <<= 1;

    QVector<double> xRe(fftSize, 0.0), xIm(fftSize, 0.0);
    QVector<double> yRe(fftSize, 0.0), yIm(fftSize, 0.0);
    for (int i = 0; i < qMin(x.size(), fftSize); ++i) xRe[i] = x[i];
    for (int i = 0; i < qMin(y.size(), fftSize); ++i) yRe[i] = y[i];

    fft(xRe, xIm, false);
    fft(yRe, yIm, false);

    // Cross spectrum and phase transform
    QVector<double> cpsRe(fftSize), cpsIm(fftSize);
    for (int k = 0; k < fftSize; ++k) {
        // X * conj(Y)
        cpsRe[k] = xRe[k] * yRe[k] + xIm[k] * yIm[k];
        cpsIm[k] = xIm[k] * yRe[k] - xRe[k] * yIm[k];
        double mag = qSqrt(cpsRe[k] * cpsRe[k] + cpsIm[k] * cpsIm[k]);
        if (mag > 1e-15) {
            cpsRe[k] /= mag;
            cpsIm[k] /= mag;
        } else {
            cpsRe[k] = 0.0;
            cpsIm[k] = 0.0;
        }
    }

    // Coherence gating
    QVector<double> coh = coherence(x, y);
    if (coh.size() == fftSize) {
        for (int k = 0; k < fftSize; ++k)
            if (k < coh.size() && coh[k] < m_coherenceThreshold) {
                cpsRe[k] = 0.0;
                cpsIm[k] = 0.0;
            }
    }

    // IFFT
    fft(cpsRe, cpsIm, true);

    // Find peak
    double delay = findPeak(cpsRe);
    // Handle wrap-around
    if (delay > fftSize / 2.0) delay -= fftSize;

    m_stats.totalCorrelations++;
    m_stats.frameSize = fftSize;
    m_stats.lastDelay = delay;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;
    emit correlationCompleted(delay, 0.0, timer.elapsed());
    return delay;
}

/* ---- Reset ---- */

void Correlator3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_autoPowX.clear();
    m_autoPowY.clear();
}
