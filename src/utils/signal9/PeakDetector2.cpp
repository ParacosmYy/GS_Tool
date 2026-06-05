/**
 * @file MultiCriteriaPeakDetector.cpp
 * @brief 多准则峰值检测器实现 — 突出度与宽度分析
 */

#include "utils/signal9/MultiCriteriaPeakDetector.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

MultiCriteriaPeakDetector::MultiCriteriaPeakDetector(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 使用默认参数检测峰值 */
QVector<PeakInfo> MultiCriteriaPeakDetector::detectPeaks(const QVector<double>& signal)
{
    return detectPeaks(signal, DetectionParams{});
}

QVector<PeakInfo> MultiCriteriaPeakDetector::detectPeaks(const QVector<double>& signal,
                                               const DetectionParams& params)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    if (N < 3) return {};

    /* 第一阶段: 查找局部极大值 */
    QVector<int> candidates = findLocalMaxima(signal, params.minDistance);

    /* 第二阶段: 按最小距离抑制 */
    candidates = suppressNonMaxima(candidates, signal, params.minDistance);

    /* 第三阶段: 计算每个候选峰的突出度 */
    QVector<PeakInfo> peaks;
    for (int idx : candidates) {
        if (signal[idx] < params.minHeight) continue;

        PeakInfo peak;
        peak.index = idx;
        peak.value = signal[idx];
        peak.prominence = computeProminence(signal, idx,
                                             &peak.leftBase, &peak.rightBase);

        if (peak.prominence < params.minProminence) continue;

        /* 第四阶段: 计算宽度 */
        peak.width = computeWidth(signal, idx, peak.prominence,
                                    params.prominenceFraction);
        peak.widthHeight = peak.value - peak.prominence * params.prominenceFraction;

        if (peak.width < params.minWidth) continue;

        /* 计算尖锐度(二阶差分近似) */
        if (idx > 0 && idx < N - 1) {
            double d2 = signal[idx - 1] - 2.0 * signal[idx] + signal[idx + 1];
            peak.sharpness = qFabs(d2);
        }

        /* 计算峰面积(三角近似) */
        double baseHeight = qMin(signal[peak.leftBase], signal[peak.rightBase]);
        peak.area = 0.5 * peak.prominence * (peak.rightBase - peak.leftBase);

        /* 显著性标记 */
        double signalRange = *std::max_element(signal.begin(), signal.end())
                             - *std::min_element(signal.begin(), signal.end());
        peak.isSignificant = (peak.prominence > 0.1 * signalRange);

        peaks.append(peak);
    }

    /* 排序 */
    if (params.sortByProminence) {
        peaks = sortByProminence(peaks, true);
    }

    ++m_stats.totalSignalsProcessed;
    m_stats.totalPeaksDetected += peaks.size();
    m_stats.totalProminenceComputations += candidates.size();
    m_stats.totalWidthComputations += peaks.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSignalsProcessed > 0)
        ? m_timeSum / m_stats.totalSignalsProcessed : 0.0;

    return peaks;
}

QVector<int> MultiCriteriaPeakDetector::findLocalMaxima(const QVector<double>& signal,
                                              int minDistance) const
{
    int N = signal.size();
    QVector<int> maxima;

    for (int i = 1; i < N - 1; ++i) {
        /* 严格局部极大值: 大于左右邻居 */
        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
            maxima.append(i);
        } else if (signal[i] == signal[i - 1] && signal[i] > signal[i + 1]) {
            /* 平台处理: 向左延伸找到平台起点 */
            int start = i;
            while (start > 0 && signal[start - 1] == signal[start]) --start;
            if (start == 0 || signal[start] > signal[start - 1]) {
                maxima.append((start + i) / 2);
            }
        }
    }

    return maxima;
}

double MultiCriteriaPeakDetector::computeProminence(const QVector<double>& signal,
                                          int peakIndex,
                                          int* leftBase,
                                          int* rightBase) const
{
    int N = signal.size();
    if (peakIndex < 0 || peakIndex >= N) return 0.0;

    double peakVal = signal[peakIndex];

    /* 向左搜索基线: 找到比峰值更高的点之间的最低点 */
    int leftIdx = findLeftBase(signal, peakIndex);
    int rightIdx = findRightBase(signal, peakIndex);

    if (leftBase) *leftBase = leftIdx;
    if (rightBase) *rightBase = rightIdx;

    /* 基线高度 = 两侧基线的较大者 */
    double baseline = qMax(signal[leftIdx], signal[rightIdx]);
    return peakVal - baseline;
}

double MultiCriteriaPeakDetector::computeWidth(const QVector<double>& signal, int peakIndex,
                                     double prominence, double relHeight) const
{
    int N = signal.size();
    if (peakIndex < 0 || peakIndex >= N || prominence <= 0.0) return 0.0;

    double peakVal = signal[peakIndex];
    double level = peakVal - prominence * relHeight;

    /* 向左搜索交叉点 */
    double leftCross = peakIndex;
    for (int i = peakIndex - 1; i >= 0; --i) {
        if (signal[i] < level) {
            leftCross = interpolateCrossing(i, signal[i], i + 1, signal[i + 1], level);
            break;
        }
        if (i == 0) leftCross = 0.0;
    }

    /* 向右搜索交叉点 */
    double rightCross = peakIndex;
    for (int i = peakIndex + 1; i < N; ++i) {
        if (signal[i] < level) {
            rightCross = interpolateCrossing(i - 1, signal[i - 1], i, signal[i], level);
            break;
        }
        if (i == N - 1) rightCross = static_cast<double>(N - 1);
    }

    return rightCross - leftCross;
}

QVector<PeakInfo> MultiCriteriaPeakDetector::sortByProminence(const QVector<PeakInfo>& peaks,
                                                    bool descending) const
{
    QVector<PeakInfo> sorted = peaks;
    if (descending) {
        std::sort(sorted.begin(), sorted.end(),
                  [](const PeakInfo& a, const PeakInfo& b) {
                      return a.prominence > b.prominence;
                  });
    } else {
        std::sort(sorted.begin(), sorted.end(),
                  [](const PeakInfo& a, const PeakInfo& b) {
                      return a.prominence < b.prominence;
                  });
    }
    return sorted;
}

QVector<PeakInfo> MultiCriteriaPeakDetector::filterPeaks(const QVector<PeakInfo>& peaks,
                                               double minProminence,
                                               double minWidth) const
{
    QVector<PeakInfo> filtered;
    for (const auto& peak : peaks) {
        if (peak.prominence >= minProminence && peak.width >= minWidth) {
            filtered.append(peak);
        }
    }
    return filtered;
}

QVector<int> MultiCriteriaPeakDetector::suppressNonMaxima(const QVector<int>& candidates,
                                                const QVector<double>& signal,
                                                int minDistance) const
{
    if (candidates.isEmpty() || minDistance <= 1) return candidates;

    QVector<int> result;
    QVector<bool> suppressed(candidates.size(), false);

    /* 按信号幅度降序排列候选索引 */
    QVector<int> order(candidates.size());
    for (int i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return signal[candidates[a]] > signal[candidates[b]];
    });

    for (int idx : order) {
        if (suppressed[idx]) continue;
        result.append(candidates[idx]);

        /* 抑制距离范围内的其他候选 */
        for (int j = 0; j < candidates.size(); ++j) {
            if (j == idx) continue;
            if (qAbs(candidates[j] - candidates[idx]) < minDistance) {
                suppressed[j] = true;
            }
        }
    }

    /* 按位置排序输出 */
    std::sort(result.begin(), result.end());
    return result;
}

int MultiCriteriaPeakDetector::findLeftBase(const QVector<double>& signal, int peakIndex) const
{
    int N = signal.size();
    double peakVal = signal[peakIndex];

    int baseIdx = peakIndex;
    double minVal = peakVal;

    for (int i = peakIndex - 1; i >= 0; --i) {
        /* 如果遇到比峰值更高的点, 停止 */
        if (signal[i] > peakVal) break;
        /* 记录最低点 */
        if (signal[i] < minVal) {
            minVal = signal[i];
            baseIdx = i;
        }
    }
    return baseIdx;
}

int MultiCriteriaPeakDetector::findRightBase(const QVector<double>& signal, int peakIndex) const
{
    int N = signal.size();
    double peakVal = signal[peakIndex];

    int baseIdx = peakIndex;
    double minVal = peakVal;

    for (int i = peakIndex + 1; i < N; ++i) {
        if (signal[i] > peakVal) break;
        if (signal[i] < minVal) {
            minVal = signal[i];
            baseIdx = i;
        }
    }
    return baseIdx;
}

double MultiCriteriaPeakDetector::interpolateCrossing(int x1, double y1, int x2, double y2,
                                            double level) const
{
    if (qFabs(y2 - y1) < 1e-15) return (x1 + x2) / 2.0;
    /* 线性插值: level = y1 + (y2 - y1) * (x - x1) / (x2 - x1) */
    return x1 + (level - y1) / (y2 - y1) * (x2 - x1);
}

MultiCriteriaPeakDetector::Stats MultiCriteriaPeakDetector::stats() const
{
    return m_stats;
}

void MultiCriteriaPeakDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
