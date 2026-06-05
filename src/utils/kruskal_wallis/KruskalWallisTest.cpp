/**
 * @file KruskalWallisTest.cpp
 * @brief Kruskal-Wallis H检验实现
 */

#include "utils/kruskal_wallis/KruskalWallisTest.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <vector>

KruskalWallisTest::KruskalWallisTest(QObject* parent)
    : QObject(parent) {}

double KruskalWallisTest::test(const QVector<QVector<double>>& groups)
{
    QElapsedTimer timer;
    timer.start();

    int k = groups.size();
    m_hStatistic = 0.0;
    m_pValue     = 1.0;

    if (k < 2) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            (m_stats.totalTests > 0) ? m_timeSum / m_stats.totalTests : 0.0;
        return m_hStatistic;
    }

    auto sorted = mergeAndSort(groups);
    int totalN = static_cast<int>(sorted.size());
    if (totalN < 2) return m_hStatistic;

    auto ranks = assignRanks(sorted);
    m_hStatistic = computeH(ranks, sorted, k, totalN);
    m_pValue = chiSqPValue(m_hStatistic, k - 1.0);

    m_stats.totalTests++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(m_hStatistic, m_pValue);
    return m_hStatistic;
}

std::vector<KruskalWallisTest::ValueGroup>
KruskalWallisTest::mergeAndSort(
    const QVector<QVector<double>>& groups) const
{
    std::vector<ValueGroup> allData;
    for (int g = 0; g < groups.size(); ++g) {
        for (double v : groups[g]) {
            allData.push_back({v, g});
        }
    }
    std::sort(allData.begin(), allData.end(),
              [](const ValueGroup& a, const ValueGroup& b) {
                  return a.value < b.value;
              });
    return allData;
}

std::vector<double> KruskalWallisTest::assignRanks(
    const std::vector<ValueGroup>& sorted) const
{
    int totalN = static_cast<int>(sorted.size());
    std::vector<double> ranks(totalN);
    int i = 0;
    while (i < totalN) {
        int j = i + 1;
        while (j < totalN &&
               qFuzzyCompare(sorted[j].value, sorted[i].value)) {
            ++j;
        }
        double avgRank = (i + 1.0 + j) / 2.0;
        for (int m = i; m < j; ++m) ranks[m] = avgRank;
        i = j;
    }
    return ranks;
}

double KruskalWallisTest::computeH(const std::vector<double>& ranks,
                                   const std::vector<ValueGroup>& sorted,
                                   int k, int totalN) const
{
    std::vector<double> rankSums(k, 0.0);
    std::vector<int>    groupSizes(k, 0);
    for (int idx = 0; idx < totalN; ++idx) {
        rankSums[sorted[idx].group]   += ranks[idx];
        groupSizes[sorted[idx].group] += 1;
    }

    double hStat = 0.0;
    for (int g = 0; g < k; ++g) {
        if (groupSizes[g] > 0) {
            hStat += rankSums[g] * rankSums[g] / groupSizes[g];
        }
    }
    hStat = 12.0 / (totalN * (totalN + 1.0)) * hStat - 3.0 * (totalN + 1.0);
    return hStat;
}

double KruskalWallisTest::chiSqPValue(double x, double df) const
{
    if (x <= 0.0 || df <= 0.0) return 1.0;
    return 1.0 - incompleteGamma(df / 2.0, x / 2.0);
}

double KruskalWallisTest::incompleteGamma(double a, double x) const
{
    if (x < 0.0 || a <= 0.0) return 0.0;
    if (x == 0.0) return 0.0;

    if (x < a + 1.0) {
        double sum = 1.0 / a;
        double term = 1.0 / a;
        for (int n = 1; n < 200; ++n) {
            term *= x / (a + n);
            sum += term;
            if (qAbs(term) < 1e-15 * qAbs(sum)) break;
        }
        double logPrefix = a * std::log(x) - x - gammaLn(a);
        return qBound(0.0, std::exp(logPrefix) * sum, 1.0);
    }

    double b = x + 1.0 - a;
    double c = 1e30;
    double d = 1.0 / b;
    double h = d;
    for (int n = 1; n <= 200; ++n) {
        double an = -static_cast<double>(n) * (static_cast<double>(n) - a);
        b += 2.0;
        d = an * d + b;
        if (qAbs(d) < 1e-30) d = 1e-30;
        c = b + an / c;
        if (qAbs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        double delta = d * c;
        h *= delta;
        if (qAbs(delta - 1.0) < 1e-15) break;
    }
    double logPrefix = a * std::log(x) - x - gammaLn(a);
    return qBound(0.0, 1.0 - std::exp(logPrefix) * h, 1.0);
}

double KruskalWallisTest::gammaLn(double x) const
{
    static const double coeff[7] = {
        0.99999999999980993,  676.5203681218851,   -1259.1392167224028,
        771.32342877765313, -176.61502916214059,     12.507343278686905,
        -0.13857109526572012
    };

    if (x < 0.5) {
        return std::log(M_PI / std::sin(M_PI * x)) - gammaLn(1.0 - x);
    }
    double z = x - 1.0;
    double s = coeff[0];
    for (int i = 1; i < 7; ++i) s += coeff[i] / (z + i);
    double t = z + 7.0 / 2.0;
    return 0.5 * std::log(2.0 * M_PI) + (z + 0.5) * std::log(t) - t + std::log(s);
}

void KruskalWallisTest::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}
