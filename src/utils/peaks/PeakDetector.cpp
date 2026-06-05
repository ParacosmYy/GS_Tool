/**
 * @file PeakDetector.cpp
 * @brief 峰值检测器实现
 */

#include "utils/peaks/PeakDetector.h"
#include <QtMath>
#include <algorithm>

PeakDetector::PeakDetector(QObject* parent)
    : QObject(parent), m_minProminence(0.1), m_minDistance(1), m_heightSum(0.0) {}

void PeakDetector::setMinProminence(double p) { m_minProminence = qMax(0.0, p); }
void PeakDetector::setMinDistance(int d) { m_minDistance = qMax(1, d); }

QList<PeakDetector::Peak> PeakDetector::detectPeaks(const QVector<double>& data)
{
    QList<Peak> peaks;
    if (data.size() < 3) return peaks;

    for (int i = 1; i < data.size() - 1; ++i) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            /* 计算prominence */
            double leftMin = data[i], rightMin = data[i];
            for (int j = i - 1; j >= 0; --j) {
                if (data[j] < leftMin) leftMin = data[j];
                if (data[j] > data[i]) break;
            }
            for (int j = i + 1; j < data.size(); ++j) {
                if (data[j] < rightMin) rightMin = data[j];
                if (data[j] > data[i]) break;
            }
            double prom = data[i] - qMax(leftMin, rightMin);

            if (prom >= m_minProminence) {
                /* 检查距离 */
                bool tooClose = false;
                for (const auto& p : peaks) {
                    if (i - p.index < m_minDistance) {
                        tooClose = true;
                        if (data[i] > p.value) {
                            peaks.removeOne(p);
                            tooClose = false;
                        }
                        break;
                    }
                }
                if (!tooClose) {
                    Peak pk;
                    pk.index = i;
                    pk.value = data[i];
                    pk.prominence = prom;
                    pk.isMaxima = true;
                    peaks.append(pk);
                }
            }
        }
    }

    /* 计算宽度(半高宽) */
    for (auto& pk : peaks) {
        double halfH = pk.value - pk.prominence / 2.0;
        int left = pk.index, right = pk.index;
        while (left > 0 && data[left] > halfH) --left;
        while (right < data.size() - 1 && data[right] > halfH) ++right;
        pk.width = right - left;
    }

    m_stats.totalPeaksFound += static_cast<quint64>(peaks.size());
    for (const auto& p : peaks) {
        m_heightSum += p.prominence;
        if (p.prominence > m_stats.peakPeakAmplitude) {
            m_stats.peakPeakAmplitude = p.prominence;
        }
    }
    if (m_stats.totalPeaksFound > 0) {
        m_stats.averagePeakHeight = m_heightSum
            / static_cast<double>(m_stats.totalPeaksFound);
    }

    emit peaksFound(peaks.size());
    return peaks;
}

QList<PeakDetector::Peak> PeakDetector::detectValleys(const QVector<double>& data)
{
    QVector<double> inverted;
    inverted.reserve(data.size());
    for (double v : data) inverted.append(-v);

    auto valleys = detectPeaks(inverted);
    for (auto& v : valleys) {
        v.value = -v.value;
        v.isMaxima = false;
        v.index = v.index; /* 索引不变 */
    }
    m_stats.totalValleysFound += static_cast<quint64>(valleys.size());
    return valleys;
}

void PeakDetector::resetStatistics()
{
    m_stats = Stats{};
    m_heightSum = 0.0;
}
