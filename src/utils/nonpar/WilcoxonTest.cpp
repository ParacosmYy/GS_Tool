/**
 * @file WilcoxonTest.cpp
 * @brief Wilcoxon符号秩检验实现
 */

#include "WilcoxonTest.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

WilcoxonTest::WilcoxonTest(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_statistic(0.0)
    , m_pValue(0.0)
{
}

double WilcoxonTest::test(const QVector<double>& x, const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    if (x.size() != y.size() || x.isEmpty()) {
        m_statistic = 0.0;
        m_pValue = 1.0;
        emit testCompleted(m_statistic, m_pValue);
        return m_statistic;
    }

    /* 计算配对差值 */
    QVector<double> differences;
    differences.reserve(x.size());
    for (int i = 0; i < x.size(); ++i) {
        differences.append(x[i] - y[i]);
    }

    /* 计算符号秩和 */
    m_statistic = computeSignedRankSum(differences);

    /* 计算非零差值个数 */
    int n = 0;
    for (double d : differences) {
        if (std::abs(d) > 1e-15) n++;
    }

    /* 正态近似p值 */
    m_pValue = normalApproxPValue(m_statistic, n);

    /* 统计更新 */
    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTests > 0) ? m_timeSum / m_stats.totalTests : 0.0;

    emit testCompleted(m_statistic, m_pValue);
    return m_statistic;
}

double WilcoxonTest::testOneSample(const QVector<double>& x, double median)
{
    QElapsedTimer timer;
    timer.start();

    if (x.isEmpty()) {
        m_statistic = 0.0;
        m_pValue = 1.0;
        emit testCompleted(m_statistic, m_pValue);
        return m_statistic;
    }

    /* 计算与中位数的偏差 */
    QVector<double> differences;
    differences.reserve(x.size());
    for (double v : x) {
        differences.append(v - median);
    }

    m_statistic = computeSignedRankSum(differences);

    int n = 0;
    for (double d : differences) {
        if (std::abs(d) > 1e-15) n++;
    }

    m_pValue = normalApproxPValue(m_statistic, n);

    /* 统计更新 */
    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTests > 0) ? m_timeSum / m_stats.totalTests : 0.0;

    emit testCompleted(m_statistic, m_pValue);
    return m_statistic;
}

void WilcoxonTest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_statistic = 0.0;
    m_pValue = 0.0;
}

double WilcoxonTest::computeSignedRankSum(QVector<double> differences) const
{
    /* 去除零差值并记录符号 */
    struct DiffItem {
        double absDiff;
        int    sign; // +1 or -1
    };

    QVector<DiffItem> items;
    for (double d : differences) {
        if (std::abs(d) > 1e-15) {
            items.append({std::abs(d), (d > 0) ? 1 : -1});
        }
    }

    if (items.isEmpty()) return 0.0;

    /* 按绝对值排序 */
    std::sort(items.begin(), items.end(),
              [](const DiffItem& a, const DiffItem& b) {
                  return a.absDiff < b.absDiff;
              });

    /* 分配秩次(处理并列取平均秩) */
    double wPlus = 0.0;
    int n = items.size();
    int i = 0;

    while (i < n) {
        int j = i;
        /* 找到并列组的结束位置 */
        while (j < n && std::abs(items[j].absDiff - items[i].absDiff) < 1e-15) {
            j++;
        }

        /* 平均秩次: (i+1 + j) / 2 = (i + j + 1) / 2.0 */
        double avgRank = (static_cast<double>(i) + j + 1.0) / 2.0;

        /* 累加正号的秩次 */
        for (int k = i; k < j; ++k) {
            if (items[k].sign > 0) {
                wPlus += avgRank;
            }
        }

        i = j;
    }

    return wPlus;
}

double WilcoxonTest::normalApproxPValue(double wPlus, int n) const
{
    if (n <= 0) return 1.0;
    if (n < 10) {
        /* 小样本正态近似精度有限，但作为基本实现仍可使用 */
    }

    /* 期望: E[W+] = n(n+1)/4 */
    double meanW = static_cast<double>(n) * (n + 1) / 4.0;

    /* 方差: Var[W+] = n(n+1)(2n+1)/24 */
    double varW = static_cast<double>(n) * (n + 1) * (2 * n + 1) / 24.0;

    if (varW <= 0.0) return 1.0;
    double stdDev = std::sqrt(varW);

    /* 连续性修正的Z值 */
    double z = std::abs(wPlus - meanW) - 0.5;
    z /= stdDev;

    /* 标准正态CDF近似(Abramowitz & Stegun) */
    double absZ = std::abs(z);
    double t = 1.0 / (1.0 + 0.2316419 * absZ);
    double d = 0.3989422804014327; /* 1/sqrt(2*pi) */
    double pdf = d * std::exp(-0.5 * absZ * absZ);
    double cdf = 1.0 - pdf * (0.319381530 * t
                               - 0.356563782 * t * t
                               + 1.781477937 * t * t * t
                               - 1.821255978 * t * t * t * t
                               + 1.330274429 * t * t * t * t * t);

    /* 双侧p值 */
    double p = 2.0 * (1.0 - cdf);
    return std::max(0.0, std::min(1.0, p));
}
