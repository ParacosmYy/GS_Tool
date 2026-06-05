/**
 * @file BootstrapResampler.cpp
 * @brief Bootstrap重采样实现
 */

#include "utils/bootstrap/BootstrapResampler.h"

#include <QtMath>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>

BootstrapResampler::BootstrapResampler(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

BootstrapResampler::ConfidenceInterval BootstrapResampler::computeCI(
    const QVector<double>& data, StatFunc statFunc,
    int iterations, double confidence)
{
    ConfidenceInterval ci;
    if (data.size() < 2 || iterations < 10) return ci;

    QElapsedTimer timer;
    timer.start();

    ci.level = confidence;
    ci.estimate = statFunc(data);

    QVector<double> bootstrapStats;
    bootstrapStats.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        QVector<double> sample = resample(data);
        double stat = statFunc(sample);
        bootstrapStats.append(stat);
    }

    std::sort(bootstrapStats.begin(), bootstrapStats.end());

    double alpha = 1.0 - confidence;
    int lowerIdx = qFloor(alpha / 2.0 * iterations);
    int upperIdx = qCeil((1.0 - alpha / 2.0) * iterations) - 1;
    lowerIdx = qBound(0, lowerIdx, iterations - 1);
    upperIdx = qBound(0, upperIdx, iterations - 1);

    ci.lower = bootstrapStats[lowerIdx];
    ci.upper = bootstrapStats[upperIdx];

    m_stats.totalResamples++;
    m_stats.totalIterations += iterations;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalResamples;

    return ci;
}

BootstrapResampler::ConfidenceInterval BootstrapResampler::meanCI(
    const QVector<double>& data, int iterations, double confidence)
{
    return computeCI(data,
        [](const QVector<double>& d) {
            double sum = 0;
            for (double v : d) sum += v;
            return sum / d.size();
        }, iterations, confidence);
}

BootstrapResampler::ConfidenceInterval BootstrapResampler::medianCI(
    const QVector<double>& data, int iterations, double confidence)
{
    return computeCI(data,
        [](const QVector<double>& d) {
            QVector<double> s = d;
            std::sort(s.begin(), s.end());
            return s[s.size() / 2];
        }, iterations, confidence);
}

QVector<double> BootstrapResampler::resample(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> sample(n);
    for (int i = 0; i < n; ++i) {
        int idx = QRandomGenerator::global()->bounded(n);
        sample[i] = data[idx];
    }
    return sample;
}

void BootstrapResampler::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
