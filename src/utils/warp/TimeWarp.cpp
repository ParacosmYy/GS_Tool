/**
 * @file TimeWarp.cpp
 * @brief 时间规整引擎实现
 */

#include "utils/warp/TimeWarp.h"
#include <QtMath>
#include <limits>

TimeWarp::TimeWarp(QObject* parent)
    : QObject(parent), m_bandWidth(-1), m_distSum(0.0), m_pathLenSum(0.0) {}

void TimeWarp::setBandWidth(int w) { m_bandWidth = w; }

double TimeWarp::euclideanDistance(double a, double b)
{
    double d = a - b;
    return d * d;
}

TimeWarp::DTWResult TimeWarp::compute(
    const QVector<double>& seq1, const QVector<double>& seq2)
{
    DTWResult result;
    int n = seq1.size(), m = seq2.size();
    if (n == 0 || m == 0) return result;

    /* DTW矩阵 */
    QVector<QVector<double>> dtw(n + 1, QVector<double>(m + 1,
        std::numeric_limits<double>::infinity()));
    dtw[0][0] = 0.0;

    int band = (m_bandWidth > 0) ? m_bandWidth : qMax(n, m);

    for (int i = 1; i <= n; ++i) {
        int jStart = qMax(1, i - band);
        int jEnd = qMin(m, i + band);
        for (int j = jStart; j <= jEnd; ++j) {
            double cost = euclideanDistance(seq1[i - 1], seq2[j - 1]);
            dtw[i][j] = cost + qMin(dtw[i - 1][j], qMin(dtw[i][j - 1],
                                     dtw[i - 1][j - 1]));
        }
    }

    result.distance = dtw[n][m];
    result.normalizedDistance = result.distance / qMax(n, m);

    /* 回溯路径 */
    int i = n, j = m;
    while (i > 0 && j > 0) {
        result.warpPath.prepend({i - 1, j - 1});
        double minPrev = qMin(dtw[i - 1][j], qMin(dtw[i][j - 1], dtw[i - 1][j - 1]));
        if (minPrev == dtw[i - 1][j - 1]) { --i; --j; }
        else if (minPrev == dtw[i - 1][j]) { --i; }
        else { --j; }
    }

    /* 统计 */
    ++m_stats.totalComparisons;
    m_distSum += result.normalizedDistance;
    m_stats.averageDistance = m_distSum
        / static_cast<double>(m_stats.totalComparisons);
    if (result.distance > m_stats.peakDistance) {
        m_stats.peakDistance = result.distance;
    }
    m_pathLenSum += result.warpPath.size();
    m_stats.averagePathLength = m_pathLenSum
        / static_cast<double>(m_stats.totalComparisons);

    emit comparisonComplete(result.distance);
    return result;
}

void TimeWarp::resetStatistics()
{
    m_stats = Stats{};
    m_distSum = m_pathLenSum = 0.0;
}
