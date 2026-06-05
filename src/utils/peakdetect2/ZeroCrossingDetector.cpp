/**
 * @file ZeroCrossingDetector.cpp
 * @brief 过零率与周期检测器实现 — 过零点定位+基频估计
 */

#include "ZeroCrossingDetector.h"

#include <QElapsedTimer>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

ZeroCrossingDetector::ZeroCrossingDetector(QObject* parent)
    : QObject(parent)
{
}

ZeroCrossingDetector::~ZeroCrossingDetector() = default;

// ═══════════════════════════════════════════════════════════
// 核心接口
// ═══════════════════════════════════════════════════════════

void ZeroCrossingDetector::process(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    m_crossings.clear();
    m_crossingRate = 0.0;

    if (signal.size() < 2) {
        m_stats.totalProcessings++;
        emit processingCompleted(0);
        return;
    }

    const int n = signal.size();

    /* 检测过零点: 相邻采样符号变化 */
    for (int i = 1; i < n; ++i) {
        double prev = signal[i - 1];
        double curr = signal[i];

        /* 严格符号变化(跳过零值) */
        if ((prev > 0.0 && curr <= 0.0) || (prev < 0.0 && curr >= 0.0)) {
            /* 线性插值精确定位 */
            m_crossings.append(i);
        }
    }

    /* 计算过零率 */
    m_crossingRate = static_cast<double>(m_crossings.size()) /
                     static_cast<double>(n - 1);

    /* 更新统计 */
    m_stats.totalProcessings++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalProcessings;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit processingCompleted(m_crossings.size());
}

double ZeroCrossingDetector::getCrossingRate() const
{
    return m_crossingRate;
}

double ZeroCrossingDetector::getFundamentalFreq(double sampleRate) const
{
    if (m_crossings.size() < 2 || sampleRate <= 0.0) {
        return 0.0;
    }

    /* 计算连续同方向过零的间隔(全周期) */
    QVector<double> halfPeriods;
    for (int i = 1; i < m_crossings.size(); ++i) {
        double diff = static_cast<double>(m_crossings[i] - m_crossings[i - 1]);
        halfPeriods.append(diff);
    }

    if (halfPeriods.isEmpty()) {
        return 0.0;
    }

    /* 中位数周期估计(鲁棒) */
    std::sort(halfPeriods.begin(), halfPeriods.end());
    double medianHalfPeriod = halfPeriods[halfPeriods.size() / 2];

    /* 两个半周期 = 一个完整周期 */
    double fullPeriodSamples = 2.0 * medianHalfPeriod;
    if (fullPeriodSamples <= 0.0) {
        return 0.0;
    }

    return sampleRate / fullPeriodSamples;
}

QVector<int> ZeroCrossingDetector::getCrossingIndices() const
{
    return m_crossings;
}

QVector<int> ZeroCrossingDetector::getPeriods() const
{
    QVector<int> periods;
    for (int i = 1; i < m_crossings.size(); ++i) {
        periods.append(m_crossings[i] - m_crossings[i - 1]);
    }
    return periods;
}

int ZeroCrossingDetector::crossingCount() const
{
    return m_crossings.size();
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

ZeroCrossingDetector::Stats ZeroCrossingDetector::stats() const
{
    return m_stats;
}

void ZeroCrossingDetector::resetStatistics()
{
    m_stats = Stats{};
}
