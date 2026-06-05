/**
 * @file PolynomialInterpolator.cpp
 * @brief 多项式插值引擎实现
 */

#include "PolynomialInterpolator.h"
#include <QElapsedTimer>
#include <cmath>

PolynomialInterpolator::PolynomialInterpolator(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void PolynomialInterpolator::setPoints(const QVector<double>& x,
                                        const QVector<double>& y)
{
    m_x = x;
    m_y = y;
    computeDividedDifferences();
    computeBarycentricWeights();
}

double PolynomialInterpolator::lagrange(double x) const
{
    if (m_x.isEmpty()) return 0.0;

    double result = 0.0;
    int n = m_x.size();

    for (int i = 0; i < n; ++i) {
        double basis = m_y[i];
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                double denom = m_x[i] - m_x[j];
                if (std::abs(denom) < 1e-15) continue;
                basis *= (x - m_x[j]) / denom;
            }
        }
        result += basis;
    }

    return result;
}

double PolynomialInterpolator::newton(double x) const
{
    if (m_x.isEmpty() || m_diffTable.isEmpty()) return 0.0;

    int n = m_x.size();
    double result = m_diffTable[0];

    for (int i = 1; i < n; ++i) {
        double product = 1.0;
        for (int j = 0; j < i; ++j)
            product *= (x - m_x[j]);
        result += m_diffTable[i] * product;
    }

    return result;
}

double PolynomialInterpolator::barycentric(double x) const
{
    if (m_x.isEmpty() || m_baryWeights.isEmpty()) return 0.0;

    int n = m_x.size();
    double numerator = 0.0, denominator = 0.0;

    for (int i = 0; i < n; ++i) {
        double diff = x - m_x[i];
        if (std::abs(diff) < 1e-15) return m_y[i];
        double w = m_baryWeights[i] / diff;
        numerator += w * m_y[i];
        denominator += w;
    }

    return (std::abs(denominator) < 1e-15) ? 0.0 : numerator / denominator;
}

QVector<double> PolynomialInterpolator::evaluateBatch(
    const QVector<double>& xPoints)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> results;
    results.reserve(xPoints.size());
    for (double x : xPoints)
        results.append(barycentric(x));

    m_stats.totalEvaluations += xPoints.size();
    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    emit interpolationCompleted(m_x.size(), xPoints.size());
    return results;
}

QVector<double> PolynomialInterpolator::dividedDifferences() const
{
    return m_diffTable;
}

void PolynomialInterpolator::computeDividedDifferences()
{
    int n = m_x.size();
    m_diffTable = m_y;

    for (int j = 1; j < n; ++j) {
        for (int i = n - 1; i >= j; --i) {
            double denom = m_x[i] - m_x[i - j];
            if (std::abs(denom) > 1e-15)
                m_diffTable[i] = (m_diffTable[i] - m_diffTable[i - 1]) / denom;
            else
                m_diffTable[i] = 0.0;
        }
    }
}

void PolynomialInterpolator::computeBarycentricWeights()
{
    int n = m_x.size();
    m_baryWeights.resize(n);
    for (int i = 0; i < n; ++i) {
        m_baryWeights[i] = 1.0;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                double diff = m_x[i] - m_x[j];
                if (std::abs(diff) > 1e-15)
                    m_baryWeights[i] /= diff;
            }
        }
    }
}

PolynomialInterpolator::Stats PolynomialInterpolator::stats() const
{
    return m_stats;
}

void PolynomialInterpolator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
