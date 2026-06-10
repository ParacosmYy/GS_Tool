/**
 * @file SignalSynchronizer8.cpp
 * @brief SignalSynchronizer8 实现
 *
 * 实现信号同步器：互相关峰值检测与分数延迟插值多通道对齐。
 */

#include "utils/signal269/SignalSynchronizer8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalSynchronizer8::SignalSynchronizer8(QObject *parent)
    : QObject(parent) {}

SignalSynchronizer8::~SignalSynchronizer8() = default;

/* ---- Statistics helpers ---- */

double SignalSynchronizer8::mean(const QVector<double>& signal)
{
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : signal) sum += v;
    return sum / signal.size();
}

double SignalSynchronizer8::stddev(const QVector<double>& signal)
{
    if (signal.size() < 2) return 0.0;
    double m = mean(signal);
    double sum = 0.0;
    for (double v : signal) {
        double d = v - m;
        sum += d * d;
    }
    return qSqrt(sum / (signal.size() - 1));
}

/* ---- Set reference signal ---- */

void SignalSynchronizer8::setReference(const QVector<double>& ref)
{
    m_reference = ref;
}

/* ---- Compute normalized cross-correlation ---- */

QVector<double> SignalSynchronizer8::crossCorrelation(
    const QVector<double>& a, const QVector<double>& b) const
{
    int nA = a.size();
    int nB = b.size();
    int maxLag = qMin(nA, nB);
    int corrLen = 2 * maxLag - 1;

    double meanA = mean(a);
    double meanB = mean(b);
    double sigA = stddev(a);
    double sigB = stddev(b);
    double norm = sigA * sigB * nA;
    if (norm < 1e-15) return QVector<double>(corrLen, 0.0);

    QVector<double> corr(corrLen, 0.0);
    for (int lag = -(maxLag - 1); lag < maxLag; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < nA; ++i) {
            int j = i - lag;
            if (j >= 0 && j < nB) {
                sum += (a[i] - meanA) * (b[j] - meanB);
                count++;
            }
        }
        corr[lag + maxLag - 1] = sum / norm;
    }
    return corr;
}

/* ---- Find integer peak lag ---- */

int SignalSynchronizer8::findPeakLag(const QVector<double>& signal) const
{
    if (m_reference.isEmpty() || signal.isEmpty()) return 0;

    auto corr = crossCorrelation(m_reference, signal);
    int maxLag = qMin(m_reference.size(), signal.size());
    int offset = maxLag - 1;

    double bestVal = -std::numeric_limits<double>::max();
    int bestIdx = 0;
    for (int i = 0; i < corr.size(); ++i) {
        if (corr[i] > bestVal) {
            bestVal = corr[i];
            bestIdx = i;
        }
    }
    return bestIdx - offset;
}

/* ---- Find fractional delay via parabolic interpolation ---- */

double SignalSynchronizer8::findFractionalDelay(
    const QVector<double>& signal) const
{
    int intLag = findPeakLag(signal);
    int maxLag = qMin(m_reference.size(), signal.size());
    int offset = maxLag - 1;

    auto corr = crossCorrelation(m_reference, signal);
    int peakIdx = intLag + offset;

    // Parabolic interpolation using 3 points around peak
    if (peakIdx < 1 || peakIdx >= corr.size() - 1) return static_cast<double>(intLag);

    double y0 = corr[peakIdx - 1];
    double y1 = corr[peakIdx];
    double y2 = corr[peakIdx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-15) return static_cast<double>(intLag);

    double delta = (y0 - y2) / denom;
    return static_cast<double>(intLag) + delta;
}

/* ---- Lagrange interpolation for fractional index ---- */

double SignalSynchronizer8::lagrangeInterpolate(
    const QVector<double>& signal, double fractionalIndex) const
{
    int idx = static_cast<int>(qFloor(fractionalIndex));
    if (idx < 0) return signal.isEmpty() ? 0.0 : signal.first();
    if (idx >= signal.size() - 1) return signal.last();

    // 4-point Lagrange interpolation
    double frac = fractionalIndex - idx;
    int i0 = qMax(0, idx - 1);
    int i1 = idx;
    int i2 = qMin(signal.size() - 1, idx + 1);
    int i3 = qMin(signal.size() - 1, idx + 2);

    // Cubic Lagrange using fractional position
    double p = frac;
    double result = signal[i1] * (1.0 - p) + signal[i2] * p;

    // Refine with 4-point formula if possible
    if (i0 != i1 && i3 != i2) {
        double t = frac;
        double t2 = t * t;
        double t3 = t2 * t;
        double h = 1.0;
        result = signal[i0] * (-(t - 1) * (t - 2) * (t - 3)) / 6.0 * h +
                 signal[i1] * ((t) * (t - 2) * (t - 3)) / 2.0 * h +
                 signal[i2] * (-(t) * (t - 1) * (t - 3)) / 2.0 * h +
                 signal[i3] * ((t) * (t - 1) * (t - 2)) / 6.0 * h;
    }
    return result;
}

/* ---- Align signal using detected delay ---- */

QVector<double> SignalSynchronizer8::alignSignal(
    const QVector<double>& signal, double delay) const
{
    if (signal.isEmpty()) return {};

    int n = signal.size();
    QVector<double> aligned(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double srcIdx = i - delay;
        if (srcIdx < 0.0 || srcIdx >= static_cast<double>(n - 1)) {
            aligned[i] = 0.0; // Zero-pad out of range
        } else {
            aligned[i] = lagrangeInterpolate(signal, srcIdx);
        }
    }
    return aligned;
}

/* ---- Synchronize multiple channels ---- */

QVector<QVector<double>> SignalSynchronizer8::synchronize(
    const QVector<QVector<double>>& channels) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;
    double maxDelay = 0.0;

    for (const auto& ch : channels) {
        double delay = findFractionalDelay(ch);
        maxDelay = qMax(maxDelay, qAbs(delay));
        result.append(alignSignal(ch, delay));
    }

    double elapsed = timer.elapsed();
    m_stats.numChannels = channels.size();
    m_stats.signalLength = m_reference.size();
    m_stats.maxDelay = maxDelay;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit synchronizationCompleted(channels.size(), maxDelay, elapsed);

    return result;
}

/* ---- Reset ---- */

void SignalSynchronizer8::resetStatistics()
{
    m_reference.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
