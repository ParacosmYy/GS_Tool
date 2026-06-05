/**
 * @file FriedmanTest.cpp
 * @brief Friedman检验实现
 */

#include "utils/friedman/FriedmanTest.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

FriedmanTest::FriedmanTest(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_lastChi2(0.0)
    , m_lastPValue(0.0)
{
}

double FriedmanTest::test(const QVector<QVector<double>>& groups)
{
    QElapsedTimer timer;
    timer.start();

    int n = groups.size();         /* 受试者数 */
    if (n == 0) {
        m_lastChi2 = 0.0;
        m_lastPValue = 1.0;
        return 0.0;
    }
    int k = groups[0].size();     /* 处理数 */
    if (k < 2) {
        m_lastChi2 = 0.0;
        m_lastPValue = 1.0;
        return 0.0;
    }

    /* 计算每行的秩并求和 */
    QVector<double> rankSums(k, 0.0);
    for (int i = 0; i < n; ++i) {
        if (groups[i].size() != k) continue;
        QVector<double> ranks = computeRanks(groups[i]);
        for (int j = 0; j < k; ++j) {
            rankSums[j] += ranks[j];
        }
    }

    /* 计算卡方统计量 */
    double chi2 = 0.0;
    for (int j = 0; j < k; ++j) {
        double diff = rankSums[j] - n * (k + 1.0) / 2.0;
        chi2 += diff * diff;
    }
    chi2 = 12.0 / (n * k * (k + 1.0)) * chi2;

    /* F校正: T = chi2 / (1 - sum_ties/(n*k*(k^2-1))) */
    double tieSum = 0.0;
    for (int i = 0; i < n; ++i) {
        if (groups[i].size() != k) continue;
        tieSum += computeTieCorrection(groups[i]);
    }
    double denom = 1.0 - tieSum / (n * k * (k * k - 1.0));
    if (denom > 0.0 && tieSum > 0.0) {
        chi2 /= denom;
    }

    m_lastChi2 = chi2;
    double df = k - 1.0;
    m_lastPValue = 1.0 - chiSquareCDF(chi2, df);

    /* 更新统计 */
    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(m_lastChi2, m_lastPValue);
    return m_lastChi2;
}

QVector<double> FriedmanTest::computeRanks(const QVector<double>& row) const
{
    int k = row.size();
    QVector<QPair<double, int>> sorted(k);
    for (int i = 0; i < k; ++i) {
        sorted[i] = {row[i], i};
    }
    std::sort(sorted.begin(), sorted.end());

    QVector<double> ranks(k, 0.0);
    int i = 0;
    while (i < k) {
        int j = i;
        while (j < k && sorted[j].first == sorted[i].first) ++j;
        double avgRank = (i + 1.0 + j) / 2.0;
        for (int m = i; m < j; ++m) {
            ranks[sorted[m].second] = avgRank;
        }
        i = j;
    }
    return ranks;
}

double FriedmanTest::computeTieCorrection(const QVector<double>& row) const
{
    int k = row.size();
    QVector<double> sorted = row;
    std::sort(sorted.begin(), sorted.end());
    double correction = 0.0;
    int i = 0;
    while (i < k) {
        int j = i;
        while (j < k && sorted[j] == sorted[i]) ++j;
        int tieSize = j - i;
        if (tieSize > 1) {
            correction += tieSize * tieSize * tieSize - tieSize;
        }
        i = j;
    }
    return correction;
}

double FriedmanTest::pValue() const
{
    return m_lastPValue;
}

double FriedmanTest::chiSquared() const
{
    return m_lastChi2;
}

double FriedmanTest::gammaFunction(double x) const
{
    /* Lanczos近似 */
    static const double g = 7.0;
    static const double coef[9] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028,
        771.32342877765313, -176.61502916214059, 12.507343278686905,
        -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7
    };
    if (x < 0.5) {
        return M_PI / (std::sin(M_PI * x) * gammaFunction(1.0 - x));
    }
    x -= 1.0;
    double ag = coef[0];
    for (int i = 1; i < 9; ++i) {
        ag += coef[i] / (x + i);
    }
    double t = x + g + 0.5;
    return std::sqrt(2.0 * M_PI) * std::pow(t, x + 0.5) * std::exp(-t) * ag;
}

double FriedmanTest::incompleteGamma(double s, double x) const
{
    /* 级数展开 */
    if (x < 0.0) return 0.0;
    double sum = 1.0 / s;
    double term = 1.0 / s;
    for (int i = 1; i < 200; ++i) {
        term *= x / (s + i);
        sum += term;
        if (std::fabs(term) < std::fabs(sum) * 1e-12) break;
    }
    return std::pow(x, s) * std::exp(-x) * sum;
}

double FriedmanTest::chiSquareCDF(double x, double df) const
{
    if (x <= 0.0) return 0.0;
    double s = df / 2.0;
    double gammaS = gammaFunction(s);
    if (gammaS == 0.0) return 0.0;
    return incompleteGamma(s, x / 2.0) / gammaS;
}

void FriedmanTest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
