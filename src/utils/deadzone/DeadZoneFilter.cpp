/**
 * @file DeadZoneFilter.cpp
 * @brief 死区滤波器实现
 */

#include "utils/deadzone/DeadZoneFilter.h"

#include <QElapsedTimer>

DeadZoneFilter::DeadZoneFilter(double zoneLow, double zoneHigh,
                                 QObject* parent)
    : QObject(parent), m_zoneLow(zoneLow), m_zoneHigh(zoneHigh),
      m_lastOutput(0.0), m_initialized(false), m_timeSum(0.0) {}

double DeadZoneFilter::process(double value)
{
    QElapsedTimer timer;
    timer.start();

    double output;
    if (value >= m_zoneLow && value <= m_zoneHigh) {
        output = 0.0;
        m_stats.totalFiltered++;
    } else {
        output = value;
        m_stats.totalPassed++;
    }

    m_lastOutput = output;
    m_initialized = true;

    m_stats.totalSamples++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSamples, 1ULL);

    emit sampleFiltered(value, output);
    return output;
}

QVector<double> DeadZoneFilter::processBatch(const QVector<double>& values)
{
    QVector<double> result;
    result.reserve(values.size());
    for (double v : values) result.append(process(v));
    return result;
}

void DeadZoneFilter::setZone(double low, double high)
{
    m_zoneLow = qMin(low, high);
    m_zoneHigh = qMax(low, high);
}

void DeadZoneFilter::reset()
{
    m_lastOutput = 0.0;
    m_initialized = false;
}

void DeadZoneFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
