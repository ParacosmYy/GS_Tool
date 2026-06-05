/**
 * @file KsTestCalculator.cpp
 * @brief Kolmogorov-Smirnov检验实现
 */

#include "utils/ks/KsTestCalculator.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

KsTestCalculator::KsTestCalculator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

KsTestCalculator::Result KsTestCalculator::twoSampleTest(
    const QVector<double>& sample1, const QVector<double>& sample2)
{
    Result r;
    if (sample1.isEmpty() || sample2.isEmpty()) return r;

    QElapsedTimer timer;
    timer.start();

    QVector<double> s1 = sample1, s2 = sample2;
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());

    int n1 = s1.size(), n2 = s2.size();
    int i = 0, j = 0;
    double d = 0.0;

    while (i < n1 && j < n2) {
        double f1 = static_cast<double>(i) / n1;
        double f2 = static_cast<double>(j) / n2;
        d = qMax(d, qAbs(f1 - f2));

        if (s1[i] < s2[j]) ++i;
        else if (s1[i] > s2[j]) ++j;
        else { ++i; ++j; }
    }
    d = qMax(d, qAbs(static_cast<double>(i) / n1 - static_cast<double>(j) / n2));

    r.dStatistic = d;
    r.pValue = computePValue(d, n1, n2);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.dStatistic, r.pValue);
    return r;
}

KsTestCalculator::Result KsTestCalculator::oneSampleUniform(const QVector<double>& data)
{
    Result r;
    if (data.isEmpty()) return r;

    QElapsedTimer timer;
    timer.start();

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    double d = 0.0;
    for (int i = 0; i < n; ++i) {
        double f = static_cast<double>(i + 1) / n;
        d = qMax(d, qAbs(f - sorted[i]));
        d = qMax(d, qAbs(static_cast<double>(i) / n - sorted[i]));
    }

    r.dStatistic = d;
    double neff = qSqrt(static_cast<double>(n));
    double lambda = (neff + 0.12 + 0.11 / neff) * d;
    r.pValue = 1.0 - kolmogorovCdf(lambda);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.dStatistic, r.pValue);
    return r;
}

KsTestCalculator::Result KsTestCalculator::oneSampleNormal(const QVector<double>& data)
{
    /* 简化: 先标准化再按均匀分布检验 */
    if (data.size() < 3) return Result{};

    double sum = 0, sumSq = 0;
    for (double v : data) { sum += v; sumSq += v * v; }
    double mean = sum / data.size();
    double stdDev = qSqrt(sumSq / data.size() - mean * mean);
    if (stdDev < 1e-15) return Result{};

    QVector<double> standardized;
    standardized.reserve(data.size());
    for (double v : data) standardized.append((v - mean) / stdDev);

    return oneSampleUniform(standardized);
}

double KsTestCalculator::computePValue(double d, int n1, int n2)
{
    double neff = qSqrt(static_cast<double>(n1 * n2) / (n1 + n2));
    double lambda = (neff + 0.12 + 0.11 / neff) * d;
    return 1.0 - kolmogorovCdf(lambda);
}

double KsTestCalculator::kolmogorovCdf(double x)
{
    if (x <= 0) return 0.0;
    if (x > 4.0) return 1.0;

    double sum = 0.0;
    for (int k = 1; k <= 100; ++k) {
        double term = qExp(-2.0 * k * k * x * x);
        if (k % 2 == 0) sum -= term;
        else sum += term;
    }
    return qBound(0.0, 2.0 * sum, 1.0);
}

void KsTestCalculator::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
