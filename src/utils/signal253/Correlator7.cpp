/**
 * @file Correlator7.cpp
 * @brief Correlator7 实现
 *
 * 实现互相关器：GCC-PHAT广义互相关相位变换与相干加权时延估计。
 */

#include "utils/signal253/Correlator7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Correlator7::Correlator7(QObject *parent) : QObject(parent) {}
Correlator7::~Correlator7() = default;

void Correlator7::setFFTSize(int size)
{
    int s = 256;
    while (s < size) s <<= 1;
    m_fftSize = s;
}

void Correlator7::setMaxLag(int maxLag) { m_maxLag = qMax(1, maxLag); }

/* ---- Radix-2 FFT ---- */

void Correlator7::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
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

/* ---- Radix-2 IFFT ---- */

void Correlator7::ifft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (auto& v : im) v = -v;
    fft(re, im);
    for (auto& v : im) v = -v;
    for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
}

/* ---- Zero-pad signal ---- */

QVector<double> Correlator7::padSignal(const QVector<double>& sig) const
{
    QVector<double> padded(m_fftSize, 0.0);
    for (int i = 0; i < qMin(sig.size(), m_fftSize); ++i)
        padded[i] = sig[i];
    return padded;
}

/* ---- Cross-spectrum with PHAT weighting ---- */

void Correlator7::crossSpectrumPHAT(const QVector<double>& xRe,
                                       const QVector<double>& xIm,
                                       const QVector<double>& yRe,
                                       const QVector<double>& yIm,
                                       QVector<double>& outRe,
                                       QVector<double>& outIm) const
{
    int n = xRe.size();
    outRe.resize(n);
    outIm.resize(n);
    for (int k = 0; k < n; ++k) {
        // Cross-spectrum: X * conj(Y)
        double crossRe = xRe[k] * yRe[k] + xIm[k] * yIm[k];
        double crossIm = xIm[k] * yRe[k] - xRe[k] * yIm[k];
        double mag = qSqrt(crossRe * crossRe + crossIm * crossIm);
        // PHAT weighting: normalize by magnitude
        if (mag > 1e-10) {
            outRe[k] = crossRe / mag;
            outIm[k] = crossIm / mag;
        } else {
            outRe[k] = 0.0;
            outIm[k] = 0.0;
        }
    }
}

/* ---- Find peak with parabolic interpolation ---- */

double Correlator7::findPeak(const QVector<double>& corr) const
{
    int n = corr.size();
    int maxIdx = 0;
    double maxVal = corr[0];
    int halfN = n / 2;

    // Search around zero lag
    int searchLo = halfN - m_maxLag;
    int searchHi = halfN + m_maxLag;
    for (int i = qMax(0, searchLo); i <= qMin(n - 1, searchHi); ++i) {
        if (corr[i] > maxVal) { maxVal = corr[i]; maxIdx = i; }
    }

    // Parabolic interpolation for sub-sample precision
    double y0 = (maxIdx > 0) ? corr[maxIdx - 1] : maxVal;
    double y1 = maxVal;
    double y2 = (maxIdx < n - 1) ? corr[maxIdx + 1] : maxVal;
    double delta = (y0 - 2.0 * y1 + y2);
    double refined = (qAbs(delta) > 1e-15)
        ? maxIdx + 0.5 * (y0 - y2) / delta
        : (double)maxIdx;

    // Convert to lag (center at zero)
    return refined - halfN;
}

/* ---- GCC-PHAT ---- */

QVector<double> Correlator7::gccPhat(const QVector<double>& x,
                                        const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> xPad = padSignal(x);
    QVector<double> yPad = padSignal(y);
    int n = m_fftSize;

    QVector<double> xIm(n, 0.0), yIm(n, 0.0);
    fft(xPad, xIm);
    fft(yPad, yIm);

    QVector<double> crossRe, crossIm;
    crossSpectrumPHAT(xPad, xIm, yPad, yIm, crossRe, crossIm);

    ifft(crossRe, crossIm);

    m_stats.fftSize = m_fftSize;
    m_stats.numCorrelations++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double peakLag = findPeak(crossRe);
    m_stats.lastDelay = peakLag;
    emit correlationCompleted(m_fftSize, peakLag, timer.elapsed());

    return crossRe;
}

/* ---- Estimate delay ---- */

double Correlator7::estimateDelay(const QVector<double>& x,
                                     const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> corr = gccPhat(x, y);
    double delay = findPeak(corr);

    m_stats.lastDelay = delay;
    m_timeSum += timer.elapsed();
    emit delayEstimated(delay, m_stats.lastCoherence, timer.elapsed());
    return delay;
}

/* ---- Magnitude-squared coherence ---- */

QVector<double> Correlator7::coherence(const QVector<double>& x,
                                          const QVector<double>& y)
{
    QVector<double> xPad = padSignal(x);
    QVector<double> yPad = padSignal(y);
    int n = m_fftSize;
    QVector<double> xIm(n, 0.0), yIm(n, 0.0);

    fft(xPad, xIm);
    fft(yPad, yIm);

    int bins = n / 2 + 1;
    QVector<double> pxx(bins), pyy(bins), pxy(bins);

    for (int k = 0; k < bins; ++k) {
        pxx[k] = xPad[k] * xPad[k] + xIm[k] * xIm[k];
        pyy[k] = yPad[k] * yPad[k] + yIm[k] * yIm[k];
        double crossRe = xPad[k] * yPad[k] + xIm[k] * yIm[k];
        double crossIm = xIm[k] * yPad[k] - xPad[k] * yIm[k];
        pxy[k] = crossRe * crossRe + crossIm * crossIm;
    }

    QVector<double> coh(bins);
    for (int k = 0; k < bins; ++k)
        coh[k] = (pxx[k] * pyy[k] > 1e-15) ? pxy[k] / (pxx[k] * pyy[k]) : 0.0;

    return coh;
}

/* ---- Coherence-weighted delay estimation ---- */

double Correlator7::coherenceWeightedDelay(const QVector<double>& x,
                                              const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> corr = gccPhat(x, y);
    QVector<double> coh = coherence(x, y);

    int n = corr.size();
    int halfN = n / 2;

    // Weight correlation by average coherence
    double avgCoh = 0.0;
    for (double c : coh) avgCoh += c;
    avgCoh /= coh.size();
    m_stats.lastCoherence = avgCoh;

    // Apply coherence weighting to correlation
    for (int i = 0; i < n; ++i)
        corr[i] *= avgCoh;

    double delay = findPeak(corr);
    m_stats.lastDelay = delay;
    m_timeSum += timer.elapsed();

    emit delayEstimated(delay, avgCoh, timer.elapsed());
    return delay;
}

/* ---- Reset ---- */

void Correlator7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
