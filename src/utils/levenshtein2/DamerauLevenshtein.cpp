/**
 * @file DamerauLevenshtein.cpp
 * @brief Damerau-Levenshtein距离实现
 */

#include "utils/levenshtein2/DamerauLevenshtein.h"

#include <QElapsedTimer>
#include <algorithm>

DamerauLevenshtein::DamerauLevenshtein(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

int DamerauLevenshtein::distance(const QString& a, const QString& b)
{
    QElapsedTimer timer;
    timer.start();

    int m = a.length();
    int n = b.length();

    /* 边界情况 */
    if (m == 0) {
        m_stats.totalComputed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computed(n);
        return n;
    }
    if (n == 0) {
        m_stats.totalComputed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computed(m);
        return m;
    }

    /* OSA (Optimal String Alignment) 变体的DP */
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;

            dp[i][j] = std::min({dp[i - 1][j] + 1,       /* 删除 */
                                  dp[i][j - 1] + 1,       /* 插入 */
                                  dp[i - 1][j - 1] + cost /* 替换 */
            });

            /* 相邻交换 */
            if (i > 1 && j > 1 &&
                a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + cost);
            }
        }
    }

    int result = dp[m][n];

    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computed(result);
    return result;
}

double DamerauLevenshtein::normalizedDistance(const QString& a,
                                                const QString& b)
{
    int maxLen = qMax(a.length(), b.length());
    if (maxLen == 0) return 0.0;
    int dist = distance(a, b);
    return static_cast<double>(dist) / maxLen;
}

double DamerauLevenshtein::similarity(const QString& a, const QString& b)
{
    return 1.0 - normalizedDistance(a, b);
}

void DamerauLevenshtein::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
