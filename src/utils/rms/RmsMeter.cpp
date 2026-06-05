/**
 * @file RmsMeter.cpp
 * @brief RMS功率计实现
 */

#include "utils/rms/RmsMeter.h"

#include <QtMath>
#include <QElapsedTimer>

RmsMeter::RmsMeter(QObject* parent)
    : QObject(parent), m_reference(1.0), m_timeSum(0.0) {}

void RmsMeter::setReference(double ref) { m_reference = qMax(1e-10, ref); }

RmsMeter::Measurement RmsMeter::measure(const QVector<double>& data)
{
    Measurement m;
    if (data.isEmpty()) return m;

    QElapsedTimer timer;
    timer.start();

    double sumSq = 0.0;
    double peak = 0.0;
    for (double v : data) {
        sumSq += v * v;
        peak = qMax(peak, qAbs(v));
    }

    m.rms = qSqrt(sumSq / data.size());
    m.peak = peak;
    m.rmsDb = 20.0 * std::log10(qMax(m.rms / m_reference, 1e-15));
    m.peakDb = 20.0 * std::log10(qMax(peak / m_reference, 1e-15));
    m.crestFactor = (m.rms > 1e-15) ? peak / m.rms : 0.0;

    m_stats.totalMeasurements++;
    m_stats.totalSamplesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalMeasurements;

    emit measurementCompleted(m.rmsDb, m.peakDb);
    return m;
}

QVector<double> RmsMeter::slidingRms(const QVector<double>& data, int windowSize)
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    if (n == 0 || windowSize < 1) return result;

    double sumSq = 0.0;
    for (int i = 0; i < n; ++i) {
        sumSq += data[i] * data[i];
        if (i >= windowSize) sumSq -= data[i - windowSize] * data[i - windowSize];
        int count = qMin(i + 1, windowSize);
        result[i] = qSqrt(sumSq / count);
    }
    return result;
}

void RmsMeter::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
