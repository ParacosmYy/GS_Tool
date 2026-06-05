/**
 * @file CycleDetector.cpp
 * @brief 周期检测器实现
 */

#include "utils/cycle/CycleDetector.h"
#include <QtMath>
#include <algorithm>

CycleDetector::CycleDetector(QObject* parent)
    : QObject(parent), m_minPeriod(3), m_maxPeriod(500), m_periodSum(0.0) {}

void CycleDetector::setMinPeriod(int p) { m_minPeriod = qMax(2, p); }
void CycleDetector::setMaxPeriod(int p) { m_maxPeriod = qMax(m_minPeriod + 1, p); }

CycleDetector::CycleInfo CycleDetector::detect(const QVector<double>& data)
{
    CycleInfo info;
    if (data.size() < m_maxPeriod * 2) return info;

    /* 计算自相关 */
    auto acf = computeAutocorrelation(data);

    /* 寻找第一个显著峰值 */
    int period = findFirstPeakAutocorr(acf);
    if (period < m_minPeriod) return info;

    info.period = period;
    info.frequency = 1.0 / period;
    info.confidence = (period > 0 && period < acf.size())
        ? qBound(0.0, acf[period], 1.0) : 0.0;

    /* 计算占空比和周期数 */
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= data.size();

    int crossings = 0;
    bool above = data[0] > mean;
    for (int i = 1; i < data.size(); ++i) {
        bool nowAbove = data[i] > mean;
        if (nowAbove != above) {
            ++crossings;
            above = nowAbove;
        }
    }

    info.detectedCycles = crossings / 2;
    if (info.detectedCycles > 0) {
        double highSamples = 0;
        for (double v : data) { if (v > mean) highSamples += 1.0; }
        info.dutyCycle = highSamples / data.size();
    }

    ++m_stats.totalDetections;
    m_periodSum += info.period;
    m_stats.averagePeriod = m_periodSum
        / static_cast<double>(m_stats.totalDetections);
    m_stats.totalCyclesFound += info.detectedCycles;
    if (info.frequency > m_stats.peakFrequency) {
        m_stats.peakFrequency = info.frequency;
    }

    emit cycleDetected(info.period, info.confidence);
    return info;
}

void CycleDetector::resetStatistics()
{
    m_stats = Stats{};
    m_periodSum = 0.0;
}

QVector<double> CycleDetector::computeAutocorrelation(
    const QVector<double>& data) const
{
    int n = data.size();
    int maxLag = qMin(m_maxPeriod, n / 2);
    double mean = 0.0;
    for (double v : data) mean += v;
    mean /= n;

    double var = 0.0;
    for (double v : data) { double d = v - mean; var += d * d; }
    if (var < 1e-10) return QVector<double>(maxLag, 0.0);

    QVector<double> acf(maxLag);
    for (int lag = 0; lag < maxLag; ++lag) {
        double cov = 0.0;
        for (int i = 0; i < n - lag; ++i) {
            cov += (data[i] - mean) * (data[i + lag] - mean);
        }
        acf[lag] = cov / var;
    }
    return acf;
}

int CycleDetector::findFirstPeakAutocorr(const QVector<double>& acf) const
{
    if (acf.size() < m_minPeriod + 2) return 0;

    /* 找第一个超过阈值0.3的峰值 */
    double threshold = 0.3;
    for (int i = m_minPeriod; i < acf.size() - 1; ++i) {
        if (acf[i] > acf[i - 1] && acf[i] > acf[i + 1]
            && acf[i] > threshold) {
            return i;
        }
    }

    /* 降级: 找最大值 */
    int bestIdx = m_minPeriod;
    double bestVal = acf[m_minPeriod];
    for (int i = m_minPeriod + 1; i < acf.size() - 1; ++i) {
        if (acf[i] > bestVal) { bestVal = acf[i]; bestIdx = i; }
    }
    return (bestVal > 0.1) ? bestIdx : 0;
}
