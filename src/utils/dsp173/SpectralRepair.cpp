/**
 * @file SpectralRepair.cpp
 * @brief SpectralRepair 实现
 *
 * 实现频谱修复：损坏bin检测(局部统计)、邻域插值、伪影抑制。
 */

#include "utils/dsp173/SpectralRepair.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralRepair::SpectralRepair(QObject *parent)
    : QObject(parent)
{
}

SpectralRepair::~SpectralRepair() = default;

/* ---- Configuration ---- */

void SpectralRepair::setDamageThreshold(double t) { m_threshold = qMax(1.0, t); }
void SpectralRepair::setInterpolationWidth(int w) { m_width = qMax(1, w); }
void SpectralRepair::setArtifactSuppression(double f) { m_suppression = qBound(0.0, f, 1.0); }

/* ---- Local statistics ---- */

void SpectralRepair::localStats(const QVector<double>& mag, int bin, int win,
                                 double& mean, double& stddev) const
{
    int n = mag.size();
    int lo = qMax(0, bin - win);
    int hi = qMin(n - 1, bin + win);
    int count = hi - lo + 1;

    double sum = 0.0;
    for (int i = lo; i <= hi; ++i)
        sum += mag[i];
    mean = sum / count;

    double var = 0.0;
    for (int i = lo; i <= hi; ++i) {
        double d = mag[i] - mean;
        var += d * d;
    }
    stddev = qSqrt(var / qMax(count - 1, 1));
}

/* ---- Detect damaged bins ---- */

QVector<int> SpectralRepair::detectDamagedBins(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    QVector<int> damaged;

    for (int i = 1; i < n - 1; ++i) {
        double mean, stddev;
        localStats(magnitude, i, m_width, mean, stddev);
        double deviation = qAbs(magnitude[i] - mean);
        if (deviation > m_threshold * stddev && stddev > 1e-10)
            damaged.append(i);
    }
    return damaged;
}

/* ---- Interpolate a single bin ---- */

double SpectralRepair::interpolateBin(const QVector<double>& magnitude, int bin, int width) const
{
    int n = magnitude.size();
    double sumWeight = 0.0;
    double sumValue = 0.0;

    for (int w = 1; w <= width; ++w) {
        double weight = 1.0 / w;

        int left = bin - w;
        if (left >= 0) {
            sumValue += magnitude[left] * weight;
            sumWeight += weight;
        }

        int right = bin + w;
        if (right < n) {
            sumValue += magnitude[right] * weight;
            sumWeight += weight;
        }
    }

    return (sumWeight > 0.0) ? sumValue / sumWeight : magnitude[bin];
}

/* ---- Suppress artifacts around repaired bins ---- */

void SpectralRepair::suppressArtifacts(QVector<double>& magnitude,
                                        const QVector<int>& repairedBins) const
{
    if (m_suppression <= 0.0) return;

    for (int bin : repairedBins) {
        int lo = qMax(0, bin - 2);
        int hi = qMin(magnitude.size() - 1, bin + 2);
        for (int i = lo; i <= hi; ++i) {
            double dist = qAbs(i - bin);
            double factor = 1.0 - m_suppression * qExp(-dist);
            magnitude[i] *= qMax(factor, 0.1);
        }
    }
}

/* ---- Main repair ---- */

QVector<double> SpectralRepair::repair(const QVector<double>& magnitude,
                                        const QVector<double>& phase)
{
    Q_UNUSED(phase);
    QElapsedTimer timer;
    timer.start();

    int n = magnitude.size();
    if (n == 0) return {};

    /* Detect damaged bins */
    QVector<int> damaged = detectDamagedBins(magnitude);

    /* Interpolate each damaged bin */
    QVector<double> repaired = magnitude;
    for (int bin : damaged)
        repaired[bin] = interpolateBin(magnitude, bin, m_width);

    /* Second pass: re-detect and interpolate any remaining outliers */
    QVector<int> remaining = detectDamagedBins(repaired);
    for (int bin : remaining) {
        if (!damaged.contains(bin))
            repaired[bin] = interpolateBin(repaired, bin, m_width);
    }

    /* Suppress artifacts around repaired bins */
    QVector<int> allRepaired;
    for (int b : damaged) allRepaired.append(b);
    for (int b : remaining) if (!allRepaired.contains(b)) allRepaired.append(b);
    suppressArtifacts(repaired, allRepaired);

    /* Smooth boundaries between repaired and original regions */
    for (int bin : allRepaired) {
        int lo = qMax(0, bin - 1);
        int hi = qMin(n - 1, bin + 1);
        for (int i = lo; i <= hi; ++i) {
            double alpha = 0.3;
            repaired[i] = (1.0 - alpha) * repaired[i] + alpha * magnitude[i];
        }
    }

    m_stats.totalRepairs++;
    m_stats.lastRepairedBins = allRepaired.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRepairs;

    emit repairCompleted(m_stats.lastRepairedBins);
    return repaired;
}

/* ---- Statistics ---- */

void SpectralRepair::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
