/**
 * @file LevenshteinDistance.cpp
 * @brief Levenshtein距离计算器实现
 */

#include "utils/levenshtein/LevenshteinDistance.h"

#include <QElapsedTimer>
#include <algorithm>

LevenshteinDistance::LevenshteinDistance(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

int LevenshteinDistance::compute(const QString& s1, const QString& s2)
{
    return computeWeighted(s1, s2, 1, 1, 1);
}

double LevenshteinDistance::similarity(const QString& s1, const QString& s2)
{
    int maxLen = qMax(s1.length(), s2.length());
    if (maxLen == 0) return 1.0;
    int dist = compute(s1, s2);
    return 1.0 - static_cast<double>(dist) / maxLen;
}

int LevenshteinDistance::computeWeighted(const QString& s1, const QString& s2,
                                          int insertCost, int deleteCost,
                                          int replaceCost)
{
    QElapsedTimer timer;
    timer.start();

    int m = s1.length();
    int n = s2.length();

    QVector<int> prev(n + 1, 0);
    QVector<int> curr(n + 1, 0);

    for (int j = 0; j <= n; ++j) prev[j] = j * insertCost;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i * deleteCost;
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : replaceCost;
            curr[j] = std::min({curr[j - 1] + insertCost,
                            prev[j] + deleteCost,
                            prev[j - 1] + cost});
        }
        std::swap(prev, curr);
    }

    int result = prev[n];

    m_stats.totalComputations++;
    m_stats.totalDistanceSum += result;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result);
    return result;
}

void LevenshteinDistance::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
