/**
 * @file MultiPeakDetector.cpp
 * @brief 多峰检测器 — 自适应峰值/谷值检测
 */

#include "MultiPeakDetector.h"
#include <QElapsedTimer>
#include <algorithm>

MultiPeakDetector::MultiPeakDetector(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<MultiPeakDetector::PeakInfo> MultiPeakDetector::detectPeaks(
    const QVector<double>& signal, int minDistance, double minHeight)
{
    QElapsedTimer timer;
    timer.start();

    auto peaks = detectExtrema(signal, minDistance, true);

    /* 过滤最小高度 */
    QVector<PeakInfo> filtered;
    for (const auto& p : peaks) {
        if (p.value >= minHeight) filtered.append(p);
    }

    m_stats.totalDetections++;
    m_stats.totalPeaks += filtered.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit peaksDetected(filtered.size());
    return filtered;
}

QVector<MultiPeakDetector::PeakInfo> MultiPeakDetector::detectValleys(
    const QVector<double>& signal, int minDistance, double maxHeight)
{
    QElapsedTimer timer;
    timer.start();

    auto valleys = detectExtrema(signal, minDistance, false);

    QVector<PeakInfo> filtered;
    for (const auto& v : valleys) {
        if (v.value <= maxHeight) filtered.append(v);
    }

    m_stats.totalDetections++;
    m_stats.totalValleys += filtered.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit valleysDetected(filtered.size());
    return filtered;
}

QVector<double> MultiPeakDetector::computeProminence(
    const QVector<double>& signal, const QVector<int>& indices) const
{
    QVector<double> prominences;
    prominences.reserve(indices.size());

    for (int idx : indices) {
        if (idx < 0 || idx >= signal.size()) {
            prominences.append(0.0);
            continue;
        }

        double peakVal = signal[idx];

        /* 向左搜索最低点 */
        double leftMin = peakVal;
        for (int i = idx - 1; i >= 0; --i) {
            if (signal[i] > peakVal) break;
            leftMin = qMin(leftMin, signal[i]);
        }

        /* 向右搜索最低点 */
        double rightMin = peakVal;
        for (int i = idx + 1; i < signal.size(); ++i) {
            if (signal[i] > peakVal) break;
            rightMin = qMin(rightMin, signal[i]);
        }

        double baseLine = qMax(leftMin, rightMin);
        prominences.append(peakVal - baseLine);
    }

    return prominences;
}

QVector<MultiPeakDetector::PeakInfo> MultiPeakDetector::detectExtrema(
    const QVector<double>& signal, int minDistance, bool findPeaks) const
{
    int n = signal.size();
    if (n < 3) return {};

    QVector<PeakInfo> candidates;

    for (int i = 1; i < n - 1; ++i) {
        bool isExtremum;
        if (findPeaks) {
            isExtremum = (signal[i] > signal[i - 1]) && (signal[i] > signal[i + 1]);
        } else {
            isExtremum = (signal[i] < signal[i - 1]) && (signal[i] < signal[i + 1]);
        }

        if (isExtremum) {
            PeakInfo info;
            info.index = i;
            info.value = signal[i];
            info.prominence = 0.0;
            candidates.append(info);
        }
    }

    /* 最小距离过滤 — 保留更强的极值 */
    if (minDistance > 1) {
        QVector<PeakInfo> filtered;
        /* 按幅度排序 */
        std::sort(candidates.begin(), candidates.end(),
                  [findPeaks](const PeakInfo& a, const PeakInfo& b) {
                      return findPeaks ? (a.value > b.value) : (a.value < b.value);
                  });

        QVector<bool> suppressed(n, false);
        for (const auto& cand : candidates) {
            if (suppressed[cand.index]) continue;

            filtered.append(cand);

            /* 抑制周围 */
            for (int d = -minDistance; d <= minDistance; ++d) {
                int idx = cand.index + d;
                if (idx >= 0 && idx < n) suppressed[idx] = true;
            }
        }
        candidates = filtered;
    }

    /* 按索引重新排序 */
    std::sort(candidates.begin(), candidates.end(),
              [](const PeakInfo& a, const PeakInfo& b) { return a.index < b.index; });

    /* 计算突出度 */
    QVector<int> indices;
    for (const auto& c : candidates) indices.append(c.index);
    QVector<double> prominences = computeProminence(signal, indices);

    for (int i = 0; i < candidates.size(); ++i) {
        candidates[i].prominence = prominences[i];
    }

    return candidates;
}

void MultiPeakDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
