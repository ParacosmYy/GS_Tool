/**
 * @file SignalSynchronizer2.cpp
 * @brief SignalSynchronizer2 实现
 *
 * 实现信号同步：互相关峰值检测、抛物线精化、sinc分数延迟插值、时延对齐。
 */

#include "utils/signal187/SignalSynchronizer2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalSynchronizer2::SignalSynchronizer2(QObject *parent) : QObject(parent) {}
SignalSynchronizer2::~SignalSynchronizer2() = default;

/* ---- Configuration ---- */

void SignalSynchronizer2::setSearchRange(int minDelay, int maxDelay)
{
    m_minDelay = qMax(0, minDelay);
    m_maxDelay = qMax(m_minDelay + 1, maxDelay);
}

/* ---- Sinc function ---- */

double SignalSynchronizer2::sinc(double x) const
{
    if (qAbs(x) < 1e-10) return 1.0;
    double pix = M_PI * x;
    return qSin(pix) / pix;
}

/* ---- Cross-correlation ---- */

QVector<double> SignalSynchronizer2::crossCorrelate(const QVector<double>& a,
                                                      const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    int maxLag = qMin(m_maxDelay, n - 1);
    QVector<double> corr(maxLag + 1, 0.0);

    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n - lag; ++i) {
            sum += a[i] * b[i + lag];
            count++;
        }
        corr[lag] = (count > 0) ? sum / count : 0.0;
    }
    return corr;
}

/* ---- Parabolic peak refinement ---- */

double SignalSynchronizer2::refinePeak(const QVector<double>& corr,
                                        int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= corr.size() - 1)
        return static_cast<double>(peakIdx);

    // Fit parabola through 3 points and find sub-sample peak
    double y0 = corr[peakIdx - 1];
    double y1 = corr[peakIdx];
    double y2 = corr[peakIdx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-15) return static_cast<double>(peakIdx);

    double delta = (y0 - y2) / denom;
    return static_cast<double>(peakIdx) + qBound(-0.5, delta, 0.5);
}

/* ---- Sinc interpolation ---- */

double SignalSynchronizer2::sincInterpolate(const QVector<double>& signal,
                                             double index) const
{
    int n = signal.size();
    int center = qFloor(index);
    int halfWindow = 8; // Windowed sinc kernel size

    double result = 0.0;
    for (int k = center - halfWindow; k <= center + halfWindow; ++k) {
        if (k < 0 || k >= n) continue;
        double frac = index - k;
        // Lanczos window (a=2)
        double lanczos = 1.0;
        if (qAbs(frac) > 1e-10) {
            double pix = M_PI * frac;
            lanczos = qSin(pix) * qSin(pix * 0.5) / (pix * pix * 0.5);
        }
        result += signal[k] * sinc(frac) * lanczos;
    }
    return result;
}

/* ---- Apply fractional delay ---- */

QVector<double> SignalSynchronizer2::applyDelay(const QVector<double>& signal,
                                                  double delay) const
{
    int n = signal.size();
    QVector<double> shifted(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double srcIdx = i - delay;
        if (srcIdx < 0 || srcIdx >= n - 1) {
            shifted[i] = 0.0;
        } else {
            shifted[i] = sincInterpolate(signal, srcIdx);
        }
    }
    return shifted;
}

/* ---- Main synchronize ---- */

SignalSynchronizer2::SyncResult SignalSynchronizer2::synchronize(
    const QVector<double>& reference, const QVector<double>& delayed)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;
    int n = qMin(reference.size(), delayed.size());
    if (n == 0) return result;

    // Compute cross-correlation
    result.crossCorrelation = crossCorrelate(reference, delayed);

    // Find peak in search range
    int startLag = qMax(m_minDelay, 0);
    int endLag = qMin(m_maxDelay, result.crossCorrelation.size() - 1);

    double peakVal = -1e18;
    int peakIdx = startLag;
    for (int lag = startLag; lag <= endLag; ++lag) {
        if (result.crossCorrelation[lag] > peakVal) {
            peakVal = result.crossCorrelation[lag];
            peakIdx = lag;
        }
    }

    // Refine peak with parabolic interpolation
    double refinedDelay = refinePeak(result.crossCorrelation, peakIdx);

    result.integerDelay = peakIdx;
    result.fractionalDelay = refinedDelay - peakIdx;
    result.delaySamples = refinedDelay;
    result.peakCorrelation = peakVal;

    m_stats.totalSyncs++;
    m_stats.signalLength = n;
    m_stats.detectedDelay = refinedDelay;
    m_stats.peakCorrelation = peakVal;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSyncs;

    emit syncCompleted(refinedDelay, peakVal);
    return result;
}

/* ---- Reset ---- */

void SignalSynchronizer2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
