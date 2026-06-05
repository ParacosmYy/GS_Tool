/**
 * @file EditDistance.cpp
 * @brief 编辑距离计算器实现
 */

#include "EditDistance.h"
#include <QElapsedTimer>
#include <algorithm>

EditDistance::EditDistance(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

int EditDistance::levenshtein(const QString& s1, const QString& s2) const
{
    QElapsedTimer timer;
    timer.start();

    int m = s1.size(), n = s2.size();

    /* 使用两行DP优化空间 */
    QVector<int> prev(n + 1), curr(n + 1);

    for (int j = 0; j <= n; ++j)
        prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i;
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[j] = qMin({curr[j - 1] + 1,       /* 插入 */
                            prev[j] + 1,            /* 删除 */
                            prev[j - 1] + cost});   /* 替换 */
        }
        std::swap(prev, curr);
    }

    int result = prev[n];

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result);
    return result;
}

int EditDistance::damerauLevenshtein(const QString& s1, const QString& s2) const
{
    QElapsedTimer timer;
    timer.start();

    int m = s1.size(), n = s2.size();
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = qMin({dp[i - 1][j] + 1,
                             dp[i][j - 1] + 1,
                             dp[i - 1][j - 1] + cost});

            /* 交换操作 */
            if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                dp[i][j] = qMin(dp[i][j], dp[i - 2][j - 2] + cost);
            }
        }
    }

    int result = dp[m][n];

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return result;
}

QPair<int, QVector<EditDistance::EditStep>> EditDistance::editPath(
    const QString& s1, const QString& s2) const
{
    QElapsedTimer timer;
    timer.start();

    int m = s1.size(), n = s2.size();
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = qMin({dp[i - 1][j] + 1,
                             dp[i][j - 1] + 1,
                             dp[i - 1][j - 1] + cost});
        }
    }

    /* 回溯 */
    QVector<EditStep> steps;
    int i = m, j = n;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && s1[i - 1] == s2[j - 1]) {
            steps.prepend({None, i - 1, j - 1, s1[i - 1], s2[j - 1]});
            i--; j--;
        } else if (i > 0 && j > 0 && dp[i][j] == dp[i - 1][j - 1] + 1) {
            steps.prepend({Replace, i - 1, j - 1, s1[i - 1], s2[j - 1]});
            i--; j--;
        } else if (j > 0 && dp[i][j] == dp[i][j - 1] + 1) {
            steps.prepend({Insert, i, j - 1, QChar(), s2[j - 1]});
            j--;
        } else {
            steps.prepend({Delete, i - 1, j, s1[i - 1], QChar()});
            i--;
        }
    }

    int distance = dp[m][n];

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return {distance, steps};
}

double EditDistance::normalizedDistance(const QString& s1, const QString& s2) const
{
    int dist = levenshtein(s1, s2);
    int maxLen = qMax(s1.size(), s2.size());
    return (maxLen > 0) ? static_cast<double>(dist) / maxLen : 0.0;
}

double EditDistance::similarity(const QString& s1, const QString& s2) const
{
    return 1.0 - normalizedDistance(s1, s2);
}

QPair<QString, int> EditDistance::bestMatch(const QString& target,
                                             const QVector<QString>& candidates,
                                             int maxDistance) const
{
    QString best;
    int bestDist = std::numeric_limits<int>::max();

    for (const auto& cand : candidates) {
        int d = levenshtein(target, cand);
        if (d < bestDist) {
            if (maxDistance < 0 || d <= maxDistance) {
                bestDist = d;
                best = cand;
            }
        }
    }

    return {best, bestDist};
}

EditDistance::Stats EditDistance::stats() const { return m_stats; }

void EditDistance::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
