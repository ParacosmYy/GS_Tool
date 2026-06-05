/**
 * @file PeakDetector2.cpp
 * @brief 峰值检测器V2实现 — 阈值+间距+prominences
 */

#include "utils/peakdetect2/PeakDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PeakDetector2::PeakDetector2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 检测信号峰值
 *  @param signal 输入信号
 *  @param threshold 最小峰值阈值
 *  @param minDistance 最小峰值间距
 *  @return 峰值索引列表 */
QVector<int> PeakDetector2::detect(const QVector<double>& signal,
                                   double threshold, int minDistance)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 3) { return {}; }

    QVector<int> candidates;

    /* 局部极大值检测 */
    for (int i = 1; i < n - 1; ++i) {
        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
            if (signal[i] >= threshold) {
                candidates.push_back(i);
            }
        }
    }

    /* 按间距筛选: 保留每组中最大的 */
    if (minDistance > 1 && !candidates.isEmpty()) {
        QVector<int> filtered;
        /* 按峰值高度降序排列 */
        std::sort(candidates.begin(), candidates.end(),
                  [&signal](int a, int b) { return signal[a] > signal[b]; });

        QVector<bool> suppressed(n, false);
        for (int idx : candidates) {
            if (suppressed[idx]) continue;
            filtered.push_back(idx);
            /* 抑制附近峰值 */
            int lo = qMax(0, idx - minDistance);
            int hi = qMin(n - 1, idx + minDistance);
            for (int j = lo; j <= hi; ++j) suppressed[j] = true;
        }
        std::sort(filtered.begin(), filtered.end());
        candidates = std::move(filtered);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDetections;
    m_stats.totalPeaksFound += static_cast<quint64>(candidates.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit detectionCompleted(candidates.size());
    return candidates;
}

/** @brief 计算峰值prominences
 *  @param signal 输入信号
 *  @param peaks 峰值索引列表
 *  @return prominences列表 */
QVector<double> PeakDetector2::prominences(const QVector<double>& signal,
                                           const QVector<int>& peaks)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    QVector<double> result;
    result.reserve(peaks.size());

    for (int peak : peaks) {
        if (peak < 0 || peak >= n) {
            result.push_back(0.0);
            continue;
        }
        double peakVal = signal[peak];

        /* 向左搜索: 找到不低于peakVal的最低点 */
        double leftMin = peakVal;
        for (int i = peak - 1; i >= 0; --i) {
            if (signal[i] >= peakVal) break;
            leftMin = qMin(leftMin, signal[i]);
        }

        /* 向右搜索: 找到不低于peakVal的最低点 */
        double rightMin = peakVal;
        for (int i = peak + 1; i < n; ++i) {
            if (signal[i] >= peakVal) break;
            rightMin = qMin(rightMin, signal[i]);
        }

        /* prominence = peak - max(leftBase, rightBase) */
        double baseLevel = qMax(leftMin, rightMin);
        result.push_back(peakVal - baseLevel);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDetections;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    return result;
}

/** @brief 重置统计 */
void PeakDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
