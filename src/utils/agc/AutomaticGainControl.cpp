/**
 * @file AutomaticGainControl.cpp
 * @brief 自动增益控制实现
 */

#include "utils/agc/AutomaticGainControl.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

AutomaticGainControl::AutomaticGainControl(QObject* parent)
    : QObject(parent), m_targetLevel(0.5), m_attackCoeff(0.01),
      m_releaseCoeff(0.001), m_maxGain(100.0), m_gain(1.0), m_timeSum(0.0) {}

void AutomaticGainControl::setTargetLevel(double level) { m_targetLevel = qMax(0.01, level); }
void AutomaticGainControl::setAttackTime(double ms) { m_attackCoeff = 1.0 / qMax(1.0, ms); }
void AutomaticGainControl::setReleaseTime(double ms) { m_releaseCoeff = 1.0 / qMax(1.0, ms); }
void AutomaticGainControl::setMaxGain(double maxGain) { m_maxGain = qMax(1.0, maxGain); }

double AutomaticGainControl::process(double sample)
{
    double output = sample * m_gain;
    double level = qAbs(output);

    double error = m_targetLevel - level;
    double coeff = (error > 0) ? m_attackCoeff : m_releaseCoeff;

    m_gain += error * coeff;
    m_gain = qBound(1.0 / m_maxGain, m_gain, m_maxGain);

    m_stats.totalSamplesProcessed++;
    m_stats.totalGainAdjustments++;
    return output;
}

QVector<double> AutomaticGainControl::processBatch(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(data.size());

    for (double sample : data) {
        output.append(process(sample));
    }

    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = (m_stats.totalGainAdjustments > 0)
        ? m_timeSum * 1000.0 / m_stats.totalGainAdjustments : 0.0;

    return output;
}

void AutomaticGainControl::reset() { m_gain = 1.0; }
void AutomaticGainControl::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
