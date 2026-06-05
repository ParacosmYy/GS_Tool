/**
 * @file AnomalyDetector.cpp
 * @brief 异常模式检测引擎实现 — 基线偏差/突变/尖峰/停滞
 */

#include "utils/anomaly/AnomalyDetector.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
AnomalyDetector::AnomalyDetector(QObject* parent)
    : QObject(parent)
    , m_windowSize(100)
    , m_sensitivity(0.8)
    , m_baselineMean(0.0)
    , m_baselineStddev(1.0)
    , m_prevValue(0.0)
    , m_prevSlope(0.0)
    , m_stallCount(0)
{
}

/** @brief 设置窗口大小 @param size 窗口点数 */
void AnomalyDetector::setWindowSize(int size)
{
    m_windowSize = qMax(10, size);
}

/** @brief 设置灵敏度 @param sensitivity 灵敏度 */
void AnomalyDetector::setSensitivity(double sensitivity)
{
    m_sensitivity = qBound(0.1, sensitivity, 1.0);
}

/** @brief 批量检测 @param data 数据 @return 异常事件列表 */
QList<AnomalyDetector::AnomalyEvent> AnomalyDetector::detect(
    const QVector<double>& data)
{
    QList<AnomalyEvent> events;
    resetBaseline();

    for (int i = 0; i < data.size(); ++i) {
        AnomalyEvent evt = detectPoint(data[i]);
        evt.index = i;
        if (evt.confidence > 0) {
            events.append(evt);
        }
    }
    return events;
}

/** @brief 流式检测 @param value 新值 @return 异常事件 */
AnomalyDetector::AnomalyEvent AnomalyDetector::detectPoint(double value)
{
    AnomalyEvent event;
    event.value = value;
    event.confidence = 0.0;
    event.baseline = m_baselineMean;

    m_buffer.append(value);
    ++m_stats.totalPointsProcessed;

    if (m_buffer.size() < 3) {
        updateBaseline(value);
        m_prevValue = value;
        return event;
    }

    double deviation = computeDeviation(value);
    double threshold = 3.0 - 2.0 * m_sensitivity;

    /* 基线偏差检测 */
    if (deviation > threshold) {
        event.type = AnomalyType::Baseline;
        event.confidence = qMin(1.0, deviation / (threshold * 2.0));
        event.baseline = m_baselineMean;
        ++m_stats.eventsByType[static_cast<int>(AnomalyType::Baseline)];
    }

    /* 级别突变检测 */
    if (m_buffer.size() >= 5) {
        double recentMean = 0.0;
        int cnt = qMin(5, m_buffer.size());
        for (int i = m_buffer.size() - cnt; i < m_buffer.size(); ++i) {
            recentMean += m_buffer[i];
        }
        recentMean /= cnt;

        double shift = qAbs(recentMean - m_baselineMean);
        if (shift > m_baselineStddev * threshold * 1.5 && event.confidence == 0) {
            event.type = AnomalyType::LevelShift;
            event.confidence = qMin(1.0, shift / (m_baselineStddev * 4.0));
            event.baseline = m_baselineMean;
            ++m_stats.eventsByType[static_cast<int>(AnomalyType::LevelShift)];
        }
    }

    /* 尖峰检测 */
    if (m_buffer.size() >= 2) {
        double delta = qAbs(value - m_prevValue);
        if (delta > m_baselineStddev * threshold * 2.0 && event.confidence == 0) {
            event.type = AnomalyType::Spike;
            event.confidence = qMin(1.0, delta / (m_baselineStddev * 5.0));
            event.baseline = m_prevValue;
            ++m_stats.eventsByType[static_cast<int>(AnomalyType::Spike)];
        }
    }

    /* 停滞检测 */
    if (m_buffer.size() >= 3 && qFuzzyCompare(value, m_prevValue)) {
        ++m_stallCount;
        if (m_stallCount > m_windowSize / 5 && event.confidence == 0) {
            event.type = AnomalyType::Stall;
            event.confidence = qMin(1.0, static_cast<double>(m_stallCount) / m_windowSize);
            event.baseline = value;
            ++m_stats.eventsByType[static_cast<int>(AnomalyType::Stall)];
        }
    } else {
        m_stallCount = 0;
    }

    /* 更新统计 */
    if (event.confidence > 0) {
        ++m_stats.totalEventsDetected;
        if (event.confidence > m_stats.peakConfidence) {
            m_stats.peakConfidence = event.confidence;
        }
        if (m_stats.totalPointsProcessed > 0) {
            m_stats.anomalyRate = static_cast<double>(m_stats.totalEventsDetected)
                / static_cast<double>(m_stats.totalPointsProcessed);
        }
        emit anomalyDetected(event);
    }

    updateBaseline(value);
    m_prevValue = value;
    return event;
}

/** @brief 重置基线 */
void AnomalyDetector::resetBaseline()
{
    m_buffer.clear();
    m_baselineMean = 0.0;
    m_baselineStddev = 1.0;
    m_stallCount = 0;
}

/** @brief 重置统计 */
void AnomalyDetector::resetStatistics()
{
    m_stats = Stats{};
    resetBaseline();
}

/** @brief 更新自适应基线 @param value 新值 */
void AnomalyDetector::updateBaseline(double value)
{
    if (m_buffer.size() > m_windowSize) {
        m_buffer.remove(0, m_buffer.size() - m_windowSize);
    }

    if (m_buffer.isEmpty()) {
        m_baselineMean = 0.0;
        m_baselineStddev = 1e-10;
        return;
    }

    const int n = m_buffer.size();
    double sum = 0.0;
    for (double v : m_buffer) {
        sum += v;
    }
    m_baselineMean = sum / n;

    double sqSum = 0.0;
    for (double v : m_buffer) {
        double d = v - m_baselineMean;
        sqSum += d * d;
    }
    m_baselineStddev = qSqrt(sqSum / n);
    if (m_baselineStddev < 1e-10) {
        m_baselineStddev = 1e-10;
    }
}

/** @brief 计算偏差(Z-score) @param value 值 @return Z-score */
double AnomalyDetector::computeDeviation(double value) const
{
    return qAbs(value - m_baselineMean) / m_baselineStddev;
}
