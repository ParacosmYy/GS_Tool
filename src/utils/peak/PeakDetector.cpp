/**
 * @file PeakDetector.cpp
 * @brief 峰值检测引擎实现 — 4种检测方法+显著性计算
 */

#include "utils/peak/PeakDetector.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PeakDetector::PeakDetector(QObject* parent)
    : QObject(parent)
    , m_method(DetectMethod::LocalMaximum)
    , m_minHeight(0.0)
    , m_minDistance(1)
    , m_minProminence(0.0)
    , m_heightSum(0.0)
    , m_prominenceSum(0.0)
{
}

void PeakDetector::setMethod(DetectMethod method) { m_method = method; }
void PeakDetector::setMinHeight(double height) { m_minHeight = height; }
void PeakDetector::setMinDistance(int distance) { m_minDistance = qMax(1, distance); }
void PeakDetector::setMinProminence(double prominence) { m_minProminence = prominence; }

/** @brief 检测峰值 @param data 数据 @return 峰值列表 */
QList<PeakDetector::Peak> PeakDetector::detect(const QVector<double>& data)
{
    if (data.size() < 3) return {};
    return detectInRange(data, 0, data.size() - 1);
}

/** @brief 范围检测 @param data 数据 @param start 起始 @param end 结束 @return 峰值列表 */
QList<PeakDetector::Peak> PeakDetector::detectInRange(
    const QVector<double>& data, int start, int end)
{
    if (start < 0) start = 0;
    if (end >= data.size()) end = data.size() - 1;
    if (start >= end) return {};

    QList<Peak> peaks;
    switch (m_method) {
    case DetectMethod::AmplitudeThreshold:
        peaks = detectAmplitude(data, start, end);
        break;
    case DetectMethod::LocalMaximum:
        peaks = detectLocalMax(data, start, end);
        break;
    case DetectMethod::Derivative:
        peaks = detectDerivative(data, start, end);
        break;
    case DetectMethod::Prominence:
        peaks = detectProminence(data, start, end);
        break;
    }

    peaks = mergeClosePeaks(peaks);

    ++m_stats.totalScans;
    m_stats.totalPeaksFound += static_cast<quint64>(peaks.size());
    if (peaks.size() > m_stats.maxPeaksPerScan) {
        m_stats.maxPeaksPerScan = peaks.size();
    }

    for (const auto& p : peaks) {
        m_heightSum += p.height;
        m_prominenceSum += p.prominence;
    }
    if (m_stats.totalPeaksFound > 0) {
        m_stats.peakAverageHeight = m_heightSum
            / static_cast<double>(m_stats.totalPeaksFound);
        m_stats.peakAverageProminence = m_prominenceSum
            / static_cast<double>(m_stats.totalPeaksFound);
    }

    emit peaksDetected(peaks.size());
    return peaks;
}

void PeakDetector::resetStatistics()
{
    m_stats = Stats{};
    m_heightSum = 0.0;
    m_prominenceSum = 0.0;
}

/** @brief 幅度阈值检测 @param data 数据 @param start 起始 @param end 结束 */
QList<PeakDetector::Peak> PeakDetector::detectAmplitude(
    const QVector<double>& data, int start, int end)
{
    QList<Peak> peaks;
    for (int i = start; i <= end; ++i) {
        if (data[i] >= m_minHeight) {
            Peak p;
            p.index = i;
            p.value = data[i];
            p.height = data[i];
            peaks.append(p);
        }
    }
    return peaks;
}

/** @brief 局部极大值检测 @param data 数据 @param start 起始 @param end 结束 */
QList<PeakDetector::Peak> PeakDetector::detectLocalMax(
    const QVector<double>& data, int start, int end)
{
    QList<Peak> peaks;
    for (int i = start + 1; i < end; ++i) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            if (data[i] >= m_minHeight) {
                Peak p;
                p.index = i;
                p.value = data[i];
                p.height = data[i];
                peaks.append(p);
            }
        }
    }
    return peaks;
}

/** @brief 导数零交叉检测 @param data 数据 @param start 起始 @param end 结束 */
QList<PeakDetector::Peak> PeakDetector::detectDerivative(
    const QVector<double>& data, int start, int end)
{
    QList<Peak> peaks;
    for (int i = start + 1; i <= end; ++i) {
        double d1 = data[i] - data[i - 1];
        double d2 = (i < end) ? (data[i + 1] - data[i]) : -d1;

        if (d1 > 0 && d2 <= 0 && data[i] >= m_minHeight) {
            Peak p;
            p.index = i;
            p.value = data[i];
            p.height = data[i];
            peaks.append(p);
        }
    }
    return peaks;
}

/** @brief 显著性检测 @param data 数据 @param start 起始 @param end 结束 */
QList<PeakDetector::Peak> PeakDetector::detectProminence(
    const QVector<double>& data, int start, int end)
{
    QList<Peak> peaks = detectLocalMax(data, start, end);

    for (auto& p : peaks) {
        p.prominence = computeProminence(data, p.index);
        if (p.prominence < m_minProminence) {
            p.prominence = -1.0; /* 标记为待移除 */
        }
    }

    peaks.erase(
        std::remove_if(peaks.begin(), peaks.end(),
            [](const Peak& p) { return p.prominence < 0; }),
        peaks.end());

    return peaks;
}

/** @brief 计算显著性 @param data 数据 @param peakIdx 峰索引 @return 显著性 */
double PeakDetector::computeProminence(const QVector<double>& data,
                                        int peakIdx) const
{
    double peakVal = data[peakIdx];
    int n = data.size();

    /* 向左搜索基线 */
    double leftMin = peakVal;
    for (int i = peakIdx - 1; i >= 0; --i) {
        if (data[i] > peakVal) break;
        if (data[i] < leftMin) leftMin = data[i];
    }

    /* 向右搜索基线 */
    double rightMin = peakVal;
    for (int i = peakIdx + 1; i < n; ++i) {
        if (data[i] > peakVal) break;
        if (data[i] < rightMin) rightMin = data[i];
    }

    return peakVal - qMax(leftMin, rightMin);
}

/** @brief 合并近邻峰值 @param peaks 峰值列表 @return 合并后列表 */
QList<PeakDetector::Peak> PeakDetector::mergeClosePeaks(
    const QList<Peak>& peaks) const
{
    if (peaks.size() <= 1 || m_minDistance <= 1) return peaks;

    QList<Peak> result;
    result.append(peaks.first());

    for (int i = 1; i < peaks.size(); ++i) {
        Peak& last = result.last();
        if (peaks[i].index - last.index < m_minDistance) {
            if (peaks[i].value > last.value) {
                last = peaks[i];
            }
        } else {
            result.append(peaks[i]);
        }
    }
    return result;
}
