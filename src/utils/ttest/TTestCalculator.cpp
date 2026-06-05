/**
 * @file TTestCalculator.cpp
 * @brief T检验计算器实现
 */

#include "utils/ttest/TTestCalculator.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

TTestCalculator::TTestCalculator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

TTestCalculator::Result TTestCalculator::independentTest(
    const QVector<double>& group1, const QVector<double>& group2)
{
    Result r;
    if (group1.size() < 2 || group2.size() < 2) return r;

    QElapsedTimer timer;
    timer.start();

    int n1 = group1.size(), n2 = group2.size();
    double sum1 = 0, sum2 = 0;
    for (double v : group1) sum1 += v;
    for (double v : group2) sum2 += v;
    double mean1 = sum1 / n1, mean2 = sum2 / n2;

    double var1 = 0, var2 = 0;
    for (double v : group1) var1 += (v - mean1) * (v - mean1);
    for (double v : group2) var2 += (v - mean2) * (v - mean2);
    var1 /= (n1 - 1); var2 /= (n2 - 1);

    double pooledVar = ((n1 - 1) * var1 + (n2 - 1) * var2) / (n1 + n2 - 2);
    double se = qSqrt(pooledVar * (1.0 / n1 + 1.0 / n2));

    r.tValue = (se > 0) ? (mean1 - mean2) / se : 0.0;
    r.df = n1 + n2 - 2;
    r.meanDiff = mean1 - mean2;
    r.pValue = computePValue(qAbs(r.tValue), r.df);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.tValue, r.pValue);
    return r;
}

TTestCalculator::Result TTestCalculator::pairedTest(
    const QVector<double>& before, const QVector<double>& after)
{
    Result r;
    int n = qMin(before.size(), after.size());
    if (n < 2) return r;

    QElapsedTimer timer;
    timer.start();

    QVector<double> diffs(n);
    for (int i = 0; i < n; ++i) diffs[i] = after[i] - before[i];

    double sum = 0;
    for (double d : diffs) sum += d;
    double mean = sum / n;

    double var = 0;
    for (double d : diffs) var += (d - mean) * (d - mean);
    var /= (n - 1);

    double se = qSqrt(var / n);
    r.tValue = (se > 0) ? mean / se : 0.0;
    r.df = n - 1;
    r.meanDiff = mean;
    r.pValue = computePValue(qAbs(r.tValue), r.df);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.tValue, r.pValue);
    return r;
}

TTestCalculator::Result TTestCalculator::oneSampleTest(
    const QVector<double>& data, double mu)
{
    Result r;
    if (data.size() < 2) return r;

    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    double sum = 0;
    for (double v : data) sum += v;
    double mean = sum / n;

    double var = 0;
    for (double v : data) var += (v - mean) * (v - mean);
    var /= (n - 1);

    double se = qSqrt(var / n);
    r.tValue = (se > 0) ? (mean - mu) / se : 0.0;
    r.df = n - 1;
    r.meanDiff = mean - mu;
    r.pValue = computePValue(qAbs(r.tValue), r.df);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.tValue, r.pValue);
    return r;
}

double TTestCalculator::computePValue(double t, double df)
{
    double x = df / (df + t * t);
    return incompleteBeta(df / 2.0, 0.5, x);
}

double TTestCalculator::incompleteBeta(double a, double b, double x)
{
    /* 简化的不完全Beta函数 — 使用连续分数近似 */
    if (x <= 0) return 1.0;
    if (x >= 1) return 0.0;

    double lnBeta = std::lgamma(a) + std::lgamma(b) - std::lgamma(a + b);
    double prefix = qExp(a * qLn(x) + b * qLn(1.0 - x) - lnBeta) / a;

    double continuedFraction = 1.0;
    double f = 1.0, c = 1.0, d = 0.0;
    for (int i = 0; i <= 100; ++i) {
        double m = static_cast<double>(i / 2);
        double numerator;
        if (i == 0) {
            numerator = 1.0;
        } else if (i % 2 == 0) {
            numerator = (m * (b - m) * x) / ((a + 2.0 * m - 1.0) * (a + 2.0 * m));
        } else {
            numerator = -((a + m) * (a + b + m) * x) /
                        ((a + 2.0 * m) * (a + 2.0 * m + 1.0));
        }
        d = 1.0 + numerator * d;
        if (qAbs(d) < 1e-30) d = 1e-30;
        d = 1.0 / d;
        c = 1.0 + numerator / c;
        if (qAbs(c) < 1e-30) c = 1e-30;
        continuedFraction *= c * d;
        if (qAbs(c * d - 1.0) < 1e-10) break;
    }
    return qBound(0.0, 1.0 - prefix * continuedFraction, 1.0);
}

void TTestCalculator::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
