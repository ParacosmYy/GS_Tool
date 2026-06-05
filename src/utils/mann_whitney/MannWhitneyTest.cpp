/**
 * @file MannWhitneyTest.cpp
 * @brief Mann-Whitney U检验实现
 */

#include "utils/mann_whitney/MannWhitneyTest.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <utility>

MannWhitneyTest::MannWhitneyTest(QObject* parent)
    : QObject(parent) {}

double MannWhitneyTest::test(const QVector<double>& group1,
                             const QVector<double>& group2)
{
    QElapsedTimer timer;
    timer.start();

    int n1 = group1.size();
    int n2 = group2.size();

    if (n1 == 0 || n2 == 0) {
        m_uStatistic = 0.0;
        m_pValue     = 1.0;
        m_effectSize = 0.0;
        return m_uStatistic;
    }

    /* 合并两组数据 */
    QVector<double> combined;
    combined.reserve(n1 + n2);
    combined.append(group1);
    combined.append(group2);

    auto ranks = computeRanks(combined, n1);
    double r1Sum = ranks.first;
    double r2Sum = ranks.second;

    /* 计算U统计量 */
    double u1 = r1Sum - n1 * (n1 + 1) / 2.0;
    double u2 = r2Sum - n2 * (n2 + 1) / 2.0;
    m_uStatistic = qMin(u1, u2);

    /* 正态近似 */
    int N = n1 + n2;
    double meanU = n1 * n2 / 2.0;

    /* 修正并列值的方差 */
    QVector<int> tiedCounts;
    QVector<int> sortedIdx(N);
    std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
    std::sort(sortedIdx.begin(), sortedIdx.end(),
              [&](int a, int b) { return combined[a] < combined[b]; });

    int i = 0;
    while (i < N) {
        int j = i + 1;
        while (j < N && qFuzzyCompare(combined[sortedIdx[j]],
                                       combined[sortedIdx[i]])) {
            ++j;
        }
        if (j - i > 1) tiedCounts.append(j - i);
        i = j;
    }

    double tieCorrection = 0.0;
    for (int t : tiedCounts) {
        double ti = static_cast<double>(t);
        tieCorrection += (ti * ti * ti - ti);
    }

    double varU = n1 * n2 * (N * N * N - N - tieCorrection) /
                  (12.0 * N * (N - 1.0));

    if (varU <= 0.0) varU = 1e-15;

    double z = (m_uStatistic - meanU) / std::sqrt(varU);
    m_pValue = 2.0 * (1.0 - normalCdf(qAbs(z)));

    /* 效应量: r = |Z| / sqrt(N) */
    m_effectSize = qAbs(z) / std::sqrt(static_cast<double>(N));

    /* 更新统计 */
    m_stats.totalTests++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(m_uStatistic, m_pValue);
    return m_uStatistic;
}

QPair<double, double> MannWhitneyTest::computeRanks(
    const QVector<double>& combined, int n1) const
{
    int N = combined.size();
    QVector<int> idx(N);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),
              [&](int a, int b) { return combined[a] < combined[b]; });

    QVector<double> ranks(N);
    int i = 0;
    while (i < N) {
        int j = i + 1;
        while (j < N && qFuzzyCompare(combined[idx[j]], combined[idx[i]])) {
            ++j;
        }
        double avgRank = (i + 1.0 + j) / 2.0;
        for (int k = i; k < j; ++k) {
            ranks[idx[k]] = avgRank;
        }
        i = j;
    }

    double r1 = 0.0, r2 = 0.0;
    for (int k = 0; k < n1; ++k) r1 += ranks[k];
    for (int k = n1; k < N; ++k) r2 += ranks[k];
    return qMakePair(r1, r2);
}

double MannWhitneyTest::normalCdf(double z) const
{
    /* Abramowitz & Stegun 近似, 精度 ~1.5e-7 */
    double a1 =  0.254829592;
    double a2 = -0.284496736;
    double a3 =  1.421413741;
    double a4 = -1.453152027;
    double a5 =  1.061405429;
    double p  =  0.3275911;

    int sign = (z < 0) ? -1 : 1;
    z = qAbs(z) / qSqrt(2.0);
    double t = 1.0 / (1.0 + p * z);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) *
               t * std::exp(-z * z);
    return 0.5 * (1.0 + sign * y);
}

void MannWhitneyTest::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}
