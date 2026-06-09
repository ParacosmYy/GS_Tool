/**
 * @file SignalSynchronizer7.cpp
 * @brief SignalSynchronizer7 实现
 *
 * 实现信号同步器：广义互相关GCC与抛物线峰值拟合亚采样插值。
 */

#include "utils/signal255/SignalSynchronizer7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalSynchronizer7::SignalSynchronizer7(QObject *parent)
    : QObject(parent) {}
SignalSynchronizer7::~SignalSynchronizer7() = default;

/* ---- Configuration ---- */

void SignalSynchronizer7::setWeighting(GCCWeighting w)
{
    m_weighting = w;
}

/* ---- Next power of 2 ---- */

int SignalSynchronizer7::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Simple radix-2 FFT ---- */

void SignalSynchronizer7::fft(QVector<double>& re, QVector<double>& im,
                               bool inverse) const
{
    int n = re.size();
    if (n <= 1) return;

    // Bit-reversal
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (i < rev) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }

    // Butterfly
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double ang = sign * 2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tRe = cRe * re[o] - cIm * im[o];
                double tIm = cRe * im[o] + cIm * re[o];
                re[o] = re[e] - tRe;
                im[o] = im[e] - tIm;
                re[e] += tRe;
                im[e] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                cIm = cRe * wIm + cIm * wRe;
                cRe = nRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
    }
}

/* ---- Compute cross-power spectrum ---- */

void SignalSynchronizer7::crossPowerSpectrum(
    const QVector<double>& ref, const QVector<double>& del,
    QVector<double>& powerRe, QVector<double>& powerIm) const
{
    int n = ref.size();
    powerRe.resize(n);
    powerIm.resize(n);

    // FFT of reference
    QVector<double> refRe(n, 0.0), refIm(n, 0.0);
    for (int i = 0; i < n; ++i) refRe[i] = ref[i];
    fft(refRe, refIm, false);

    // FFT of delayed
    QVector<double> delRe(n, 0.0), delIm(n, 0.0);
    for (int i = 0; i < n; ++i) delRe[i] = del[i];
    fft(delRe, delIm, false);

    // Cross-power: conj(Ref) * Del
    for (int k = 0; k < n; ++k) {
        powerRe[k] = refRe[k] * delRe[k] + refIm[k] * delIm[k];
        powerIm[k] = refIm[k] * delRe[k] - refRe[k] * delIm[k];
    }
}

/* ---- Apply GCC weighting ---- */

void SignalSynchronizer7::applyWeighting(QVector<double>& re,
                                          QVector<double>& im) const
{
    int n = re.size();

    switch (m_weighting) {
    case Standard:
        // No weighting
        break;
    case PHAT: {
        // Phase Transform: normalize by magnitude
        for (int k = 0; k < n; ++k) {
            double mag = qSqrt(re[k] * re[k] + im[k] * im[k]);
            if (mag > 1e-10) { re[k] /= mag; im[k] /= mag; }
        }
        break;
    }
    case Scott: {
        // SCOT: weight by sqrt of auto-power spectra
        // Simplified: use magnitude of cross-power as weight
        for (int k = 0; k < n; ++k) {
            double mag = qSqrt(re[k] * re[k] + im[k] * im[k]);
            if (mag > 1e-10) {
                double w = qSqrt(mag);
                re[k] /= w;
                im[k] /= w;
            }
        }
        break;
    }
    case ML: {
        // Maximum Likelihood: simplified ML weight
        for (int k = 0; k < n; ++k) {
            double mag2 = re[k] * re[k] + im[k] * im[k];
            if (mag2 > 1e-10) {
                double w = mag2 / (mag2 + 1e-6);
                re[k] *= w;
                im[k] *= w;
            }
        }
        break;
    }
    }
}

/* ---- Find peak in correlation ---- */

int SignalSynchronizer7::findPeak(const QVector<double>& corr) const
{
    int n = corr.size();
    int peakIdx = 0;
    double peakVal = -std::numeric_limits<double>::max();

    for (int i = 0; i < n; ++i) {
        if (corr[i] > peakVal) {
            peakVal = corr[i];
            peakIdx = i;
        }
    }
    return peakIdx;
}

/* ---- Parabolic sub-sample peak interpolation ---- */

double SignalSynchronizer7::parabolicPeak(const QVector<double>& corr,
                                            int peakIdx) const
{
    int n = corr.size();
    if (peakIdx <= 0 || peakIdx >= n - 1) return static_cast<double>(peakIdx);

    // Fit parabola through (peakIdx-1, peakIdx, peakIdx+1)
    double y0 = corr[peakIdx - 1];
    double y1 = corr[peakIdx];
    double y2 = corr[peakIdx + 1];

    // Sub-sample offset: delta = 0.5 * (y0 - y2) / (y0 - 2*y1 + y2)
    double denom = y0 - 2.0 * y1 + y2;
    if (qFuzzyIsNull(denom)) return static_cast<double>(peakIdx);

    double delta = 0.5 * (y0 - y2) / denom;
    return static_cast<double>(peakIdx) + delta;
}

/* ---- Main synchronize ---- */

double SignalSynchronizer7::synchronize(
    const QVector<double>& reference, const QVector<double>& delayed)
{
    QElapsedTimer timer;
    timer.start();

    int n1 = reference.size();
    int n2 = delayed.size();
    if (n1 == 0 || n2 == 0) return 0.0;

    // Zero-pad to next power of 2
    int fftSize = nextPow2(n1 + n2);
    m_corrSize = fftSize;

    QVector<double> refPadded(fftSize, 0.0);
    QVector<double> delPadded(fftSize, 0.0);
    for (int i = 0; i < n1; ++i) refPadded[i] = reference[i];
    for (int i = 0; i < n2; ++i) delPadded[i] = delayed[i];

    // Cross-power spectrum
    QVector<double> cpRe, cpIm;
    crossPowerSpectrum(refPadded, delPadded, cpRe, cpIm);

    // Apply GCC weighting
    applyWeighting(cpRe, cpIm);

    // Inverse FFT to get correlation
    fft(cpRe, cpIm, true);

    // Extract real part as correlation (fftshift equivalent)
    m_corr.resize(fftSize);
    for (int i = 0; i < fftSize; ++i)
        m_corr[i] = cpRe[i];

    // Find integer peak
    int peakIdx = findPeak(m_corr);
    m_integerDelay = static_cast<double>(peakIdx);
    m_peakCorr = m_corr[peakIdx];

    // Sub-sample refinement via parabolic interpolation
    m_subSampleDelay = parabolicPeak(m_corr, peakIdx);

    // Convert to signed delay (handle circular wrap-around)
    double delay = m_subSampleDelay;
    if (delay > fftSize / 2.0)
        delay -= fftSize;

    m_stats.signalLength = qMax(n1, n2);
    m_stats.numSynchronizations++;
    m_stats.lastDelay = delay;
    m_stats.lastCorrelation = m_peakCorr;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit synchronizationCompleted(delay, m_peakCorr, timer.elapsed());
    return delay;
}

/* ---- Accessors ---- */

QVector<double> SignalSynchronizer7::correlationFunction() const
{
    return m_corr;
}

double SignalSynchronizer7::subSampleDelay() const
{
    return m_subSampleDelay;
}

/* ---- Reset ---- */

void SignalSynchronizer7::resetStatistics()
{
    m_corr.clear();
    m_corrSize = 0;
    m_integerDelay = 0.0;
    m_subSampleDelay = 0.0;
    m_peakCorr = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
