/**
 * @file Correlator5.cpp
 * @brief Correlator5 实现
 *
 * 实现互相关器：频域GCC-PHAT、相干性门控干扰抑制、时延估计。
 */

#include "utils/signal225/Correlator5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Correlator5::Correlator5(QObject *parent) : QObject(parent) {}
Correlator5::~Correlator5() = default;

/* ---- Configuration ---- */

void Correlator5::setParameters(int fftSize, double coherenceThreshold)
{
    m_fftSize = qMax(64, fftSize);
    m_coherenceThreshold = qBound(0.0, coherenceThreshold, 1.0);
}

/* ---- FFT (radix-2) ---- */

void Correlator5::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
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

/* ---- IFFT ---- */

void Correlator5::ifft(QVector<double>& re, QVector<double>& im) const
{
    for (auto& v : im) v = -v;
    fft(re, im);
    double n = re.size();
    for (int i = 0; i < (int)n; ++i) { re[i] /= n; im[i] = -im[i] / n; }
}

/* ---- Cross-power spectrum ---- */

void Correlator5::crossSpectrum(const QVector<double>& re1, const QVector<double>& im1,
                                  const QVector<double>& re2, const QVector<double>& im2,
                                  QVector<double>& crossRe, QVector<double>& crossIm) const
{
    int n = qMin(re1.size(), re2.size());
    crossRe.resize(n); crossIm.resize(n);
    for (int k = 0; k < n; ++k) {
        // S12 = S1 * conj(S2)
        crossRe[k] = re1[k] * re2[k] + im1[k] * im2[k];
        crossIm[k] = im1[k] * re2[k] - re1[k] * im2[k];
    }
}

/* ---- Apply PHAT weighting ---- */

void Correlator5::applyPhat(QVector<double>& crossRe, QVector<double>& crossIm) const
{
    for (int k = 0; k < crossRe.size(); ++k) {
        double mag = qSqrt(crossRe[k] * crossRe[k] + crossIm[k] * crossIm[k]);
        if (mag > 1e-10) {
            crossRe[k] /= mag;
            crossIm[k] /= mag;
        }
    }
}

/* ---- Apply coherence gating ---- */

void Correlator5::applyCoherenceGate(QVector<double>& crossRe, QVector<double>& crossIm,
                                       const QVector<double>& coh) const
{
    for (int k = 0; k < qMin(crossRe.size(), coh.size()); ++k) {
        double gate = (coh[k] >= m_coherenceThreshold) ? 1.0 : 0.0;
        crossRe[k] *= gate;
        crossIm[k] *= gate;
    }
}

/* ---- GCC-PHAT ---- */

Correlator5::CorrResult Correlator5::gccPhat(const QVector<double>& signal1,
                                                const QVector<double>& signal2)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_fftSize;
    CorrResult result;

    // Zero-pad signals to FFT size
    QVector<double> re1(n, 0.0), im1(n, 0.0);
    QVector<double> re2(n, 0.0), im2(n, 0.0);
    for (int i = 0; i < qMin(signal1.size(), n); ++i) re1[i] = signal1[i];
    for (int i = 0; i < qMin(signal2.size(), n); ++i) re2[i] = signal2[i];

    // Forward FFT
    fft(re1, im1);
    fft(re2, im2);

    // Compute cross-spectrum
    QVector<double> crossRe, crossIm;
    crossSpectrum(re1, im1, re2, im2, crossRe, crossIm);

    // Compute coherence for gating
    QVector<double> coh = coherence(signal1, signal2);

    // Apply coherence gating to reject interference
    applyCoherenceGate(crossRe, crossIm, coh);

    // Apply PHAT weighting
    applyPhat(crossRe, crossIm);

    // IFFT to get correlation
    ifft(crossRe, crossIm);
    result.correlation.resize(n);
    for (int i = 0; i < n; ++i)
        result.correlation[i] = crossRe[i];

    // Find peak
    double peakVal = -std::numeric_limits<double>::max();
    int peakIdx = 0;
    for (int i = 0; i < n; ++i) {
        if (result.correlation[i] > peakVal) {
            peakVal = result.correlation[i];
            peakIdx = i;
        }
    }

    // Handle wrap-around for negative lags
    if (peakIdx > n / 2) peakIdx -= n;

    result.peakLag = peakIdx;
    result.peakValue = peakVal;
    result.delaySamples = static_cast<double>(peakIdx);

    // Compute overall coherence at peak
    result.coherence = (coh.size() > qAbs(peakIdx))
                           ? coh[qAbs(peakIdx)] : 0.0;
    result.snr = (peakVal > 1e-10) ? 20.0 * qLn(peakVal) / qLn(10.0) : -100.0;

    m_stats.signalLength = qMax(signal1.size(), signal2.size());
    m_stats.fftSize = n;
    m_stats.peakLag = peakIdx;
    m_stats.peakValue = peakVal;
    m_stats.coherence = result.coherence;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit correlationCompleted(peakIdx, peakVal, timer.elapsed());
    return result;
}

/* ---- Time-domain cross-correlation ---- */

QVector<double> Correlator5::crossCorrelate(const QVector<double>& signal1,
                                              const QVector<double>& signal2) const
{
    int n1 = signal1.size(), n2 = signal2.size();
    int resultLen = n1 + n2 - 1;
    QVector<double> result(resultLen, 0.0);

    for (int lag = -(n2 - 1); lag < n1; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n2; ++i) {
            int idx = lag + i;
            if (idx >= 0 && idx < n1)
                sum += signal1[idx] * signal2[i];
        }
        result[lag + n2 - 1] = sum;
    }
    return result;
}

/* ---- Magnitude-squared coherence ---- */

QVector<double> Correlator5::coherence(const QVector<double>& signal1,
                                          const QVector<double>& signal2)
{
    int n = m_fftSize;
    QVector<double> re1(n, 0.0), im1(n, 0.0);
    QVector<double> re2(n, 0.0), im2(n, 0.0);
    for (int i = 0; i < qMin(signal1.size(), n); ++i) re1[i] = signal1[i];
    for (int i = 0; i < qMin(signal2.size(), n); ++i) re2[i] = signal2[i];

    fft(re1, im1);
    fft(re2, im2);

    int halfN = n / 2 + 1;
    QVector<double> coh(halfN);

    for (int k = 0; k < halfN; ++k) {
        double psd1 = re1[k] * re1[k] + im1[k] * im1[k];
        double psd2 = re2[k] * re2[k] + im2[k] * im2[k];
        double crossRe = re1[k] * re2[k] + im1[k] * im2[k];
        double crossIm = im1[k] * re2[k] - re1[k] * im2[k];
        double csdMag = crossRe * crossRe + crossIm * crossIm;
        double denom = psd1 * psd2;
        coh[k] = (denom > 1e-20) ? csdMag / denom : 0.0;
    }
    return coh;
}

/* ---- Estimate delay ---- */

double Correlator5::estimateDelay(const QVector<double>& correlation) const
{
    double peakVal = -std::numeric_limits<double>::max();
    int peakIdx = 0;
    int n = correlation.size();
    for (int i = 0; i < n; ++i) {
        if (correlation[i] > peakVal) {
            peakVal = correlation[i];
            peakIdx = i;
        }
    }
    if (peakIdx > n / 2) peakIdx -= n;
    return static_cast<double>(peakIdx);
}

/* ---- Reset ---- */

void Correlator5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
