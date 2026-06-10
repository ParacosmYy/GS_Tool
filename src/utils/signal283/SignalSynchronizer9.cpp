/**
 * @file SignalSynchronizer9.cpp
 * @brief SignalSynchronizer9 实现
 *
 * 实现信号同步器：相位相关与sinc插值子采样对齐的精密多通道同步。
 */

#include "utils/signal283/SignalSynchronizer9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

SignalSynchronizer9::SignalSynchronizer9(QObject *parent)
    : QObject(parent) {}

SignalSynchronizer9::~SignalSynchronizer9() = default;

/* ---- Configuration ---- */

void SignalSynchronizer9::setSincRadius(int radius) { m_sincRadius = qBound(4, radius, 64); }
void SignalSynchronizer9::setMaxSearchRange(int range) { m_maxRange = qBound(16, range, 8192); }

/* ---- Sinc function ---- */

double SignalSynchronizer9::sinc(double x) const
{
    if (qAbs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return qSin(px) / px;
}

/* ---- Lanczos window ---- */

double SignalSynchronizer9::lanczosWindow(double x) const
{
    if (qAbs(x) < 1e-10) return 1.0;
    double a = static_cast<double>(m_sincRadius);
    if (qAbs(x) >= a) return 0.0;
    double pax = M_PI * x / a;
    return qSin(pax) / pax;
}

/* ---- Cross-correlation (time domain, limited range) ---- */

QVector<double> SignalSynchronizer9::crossCorrelation(const QVector<double>& a,
                                                        const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    int range = qMin(m_maxRange, n / 2);
    QVector<double> corr(2 * range + 1, 0.0);

    for (int lag = -range; lag <= range; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n; ++i) {
            int j = i - lag;
            if (j >= 0 && j < b.size()) {
                sum += a[i] * b[j];
                count++;
            }
        }
        corr[lag + range] = (count > 0) ? sum / count : 0.0;
    }
    return corr;
}

/* ---- Phase correlation ---- */

QVector<double> SignalSynchronizer9::phaseCorrelation(const QVector<double>& a,
                                                        const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    int range = qMin(m_maxRange, n / 2);

    // Compute cross-correlation
    QVector<double> corr = crossCorrelation(a, b);

    // Normalize by auto-correlation energies for phase correlation
    double energyA = 0.0, energyB = 0.0;
    for (int i = 0; i < n; ++i) {
        energyA += a[i] * a[i];
        energyB += b[i] * b[i];
    }
    double norm = qSqrt(energyA * energyB);
    if (norm > 1e-15) {
        for (auto& v : corr) v /= norm;
    }
    return corr;
}

/* ---- Fractional peak via parabolic interpolation ---- */

double SignalSynchronizer9::fractionalPeak(const QVector<double>& corr, int intPeak) const
{
    if (intPeak <= 0 || intPeak >= corr.size() - 1) return 0.0;

    double y0 = corr[intPeak - 1];
    double y1 = corr[intPeak];
    double y2 = corr[intPeak + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-15) return 0.0;
    return 0.5 * (y0 - y2) / denom;
}

/* ---- Sinc interpolation for sub-sample shift ---- */

QVector<double> SignalSynchronizer9::sincInterpolate(const QVector<double>& signal,
                                                       double offset) const
{
    int n = signal.size();
    QVector<double> result(n, 0.0);
    int a = m_sincRadius;

    for (int i = 0; i < n; ++i) {
        // Target sample position in original signal coordinates
        double pos = i - offset;
        int center = static_cast<int>(qFloor(pos));
        double frac = pos - center;

        double sum = 0.0;
        for (int k = -a + 1; k <= a; ++k) {
            int idx = center + k;
            if (idx >= 0 && idx < n) {
                double t = frac - static_cast<double>(k);
                double weight = sinc(t) * lanczosWindow(t);
                sum += signal[idx] * weight;
            }
        }
        result[i] = sum;
    }
    return result;
}

/* ---- Synchronize single channel pair ---- */

SignalSynchronizer9::SyncResult SignalSynchronizer9::synchronizePair(
    const QVector<double>& reference, const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;
    int range = qMin(m_maxRange, qMin(reference.size(), signal.size()) / 2);

    // Phase correlation
    QVector<double> corr = phaseCorrelation(reference, signal);

    // Find integer peak
    int peakIdx = 0;
    double peakVal = -1e30;
    for (int i = 0; i < corr.size(); ++i) {
        if (corr[i] > peakVal) {
            peakVal = corr[i];
            peakIdx = i;
        }
    }

    int intOffset = peakIdx - range;

    // Refine with parabolic interpolation for fractional offset
    double fracOffset = fractionalPeak(corr, peakIdx);

    result.integerOffset = intOffset;
    result.fractionalOffset = fracOffset;
    result.sampleOffset = static_cast<double>(intOffset) + fracOffset;
    result.peakCorrelation = peakVal;
    result.valid = true;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit syncDone(0, result.sampleOffset, peakVal, elapsed);

    return result;
}

/* ---- Synchronize multiple channels ---- */

SignalSynchronizer9::MultiSyncResult SignalSynchronizer9::synchronizeMulti(
    const QVector<double>& reference, const QVector<QVector<double>>& channels)
{
    QElapsedTimer timer;
    timer.start();

    MultiSyncResult result;
    result.alignedRef = reference;
    int numCh = channels.size();

    double qualitySum = 0.0;
    for (int ch = 0; ch < numCh; ++ch) {
        SyncResult sr = synchronizePair(reference, channels[ch]);
        result.syncResults.append(sr);

        // Apply sub-sample shift via sinc interpolation
        if (sr.valid) {
            result.alignedChannels.append(sincInterpolate(channels[ch], sr.sampleOffset));
            qualitySum += sr.peakCorrelation;
        } else {
            result.alignedChannels.append(channels[ch]);
        }

        emit syncDone(ch, sr.sampleOffset, sr.peakCorrelation, 0.0);
    }

    result.overallQuality = (numCh > 0) ? qualitySum / numCh : 0.0;

    double elapsed = timer.elapsed();
    m_stats.numChannels = numCh;
    m_stats.signalLength = reference.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit multiSyncDone(numCh, result.overallQuality, elapsed);

    return result;
}

/* ---- Reset ---- */

void SignalSynchronizer9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
