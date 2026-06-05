/**
 * @file ChiSquareTest.cpp
 * @brief 卡方检验实现
 */

#include "utils/chisq/ChiSquareTest.h"

#include <QtMath>
#include <QElapsedTimer>
#include <cmath>

ChiSquareTest::ChiSquareTest(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

ChiSquareTest::Result ChiSquareTest::goodnessOfFit(
    const QVector<double>& observed, const QVector<double>& expected)
{
    Result r;
    if (observed.size() != expected.size() || observed.isEmpty()) return r;

    QElapsedTimer timer;
    timer.start();

    double chiSq = 0.0;
    for (int i = 0; i < observed.size(); ++i) {
        if (expected[i] > 0) {
            double diff = observed[i] - expected[i];
            chiSq += diff * diff / expected[i];
        }
    }

    r.chiSquare = chiSq;
    r.df = observed.size() - 1;
    r.pValue = chiSquarePValue(chiSq, r.df);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.chiSquare, r.pValue);
    return r;
}

ChiSquareTest::Result ChiSquareTest::independenceTest(
    const QVector<QVector<double>>& table)
{
    Result r;
    int rows = table.size();
    if (rows == 0) return r;
    int cols = table[0].size();
    if (cols == 0) return r;

    QElapsedTimer timer;
    timer.start();

    QVector<double> rowSums(rows, 0.0), colSums(cols, 0.0);
    double total = 0.0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rowSums[i] += table[i][j];
            colSums[j] += table[i][j];
            total += table[i][j];
        }
    }

    double chiSq = 0.0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double expected = (total > 0) ? rowSums[i] * colSums[j] / total : 0.0;
            if (expected > 0) {
                double diff = table[i][j] - expected;
                chiSq += diff * diff / expected;
            }
        }
    }

    r.chiSquare = chiSq;
    r.df = (rows - 1) * (cols - 1);
    r.pValue = chiSquarePValue(chiSq, r.df);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.chiSquare, r.pValue);
    return r;
}

double ChiSquareTest::chiSquarePValue(double x, double df)
{
    if (x <= 0 || df <= 0) return 1.0;
    double k = df / 2.0;
    double logGamma = std::lgamma(k);
    double logTerm = (k - 1.0) * std::log(x / 2.0) - x / 2.0 - logGamma;
    double sum = 1.0, term = 1.0;
    for (int i = 1; i <= 200; ++i) {
        term *= x / (k + i - 1.0);
        sum += term;
        if (term < 1e-15) break;
    }
    double incompleteGamma = std::exp(logTerm) * sum;
    double gammaK = std::exp(logGamma);
    return (gammaK > 0) ? qBound(0.0, 1.0 - incompleteGamma / gammaK, 1.0) : 1.0;
}

void ChiSquareTest::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
