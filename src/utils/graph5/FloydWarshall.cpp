/**
 * @file FloydWarshall.cpp
 * @brief Floyd-Warshall全源最短路径实现
 */

#include "utils/graph5/FloydWarshall.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
FloydWarshall::FloydWarshall(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 全源最短路径 */
FloydWarshall::AllPairsResult FloydWarshall::allPairsShortestPaths(
    const QVector<QVector<double>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    AllPairsResult result;
    int n = adjacency.size();
    if (n == 0) return result;

    const double INF = 1e18;
    result.distances = adjacency;
    result.next = QVector<QVector<int>>(n, QVector<int>(n, -1));

    /* 初始化next矩阵 */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                result.distances[i][j] = 0.0;
                result.next[i][j] = j;
            } else if (result.distances[i][j] < INF / 2.0) {
                result.next[i][j] = j;
            } else {
                result.distances[i][j] = INF;
            }
        }
    }

    /* Floyd-Warshall核心三重循环 */
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                double through = result.distances[i][k] + result.distances[k][j];
                if (through < result.distances[i][j]) {
                    result.distances[i][j] = through;
                    result.next[i][j] = result.next[i][k];
                }
            }
        }
    }

    /* 检测负权环 */
    result.hasNegativeCycle = false;
    for (int i = 0; i < n; ++i) {
        if (result.distances[i][i] < 0.0) {
            result.hasNegativeCycle = true;
            break;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    if (result.hasNegativeCycle) ++m_stats.negativeCyclesFound;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    emit queryCompleted(n, result.hasNegativeCycle);
    return result;
}

/** @brief 回溯路径 */
QVector<int> FloydWarshall::reconstructPath(const AllPairsResult& result,
                                             int from, int to) const
{
    if (result.next[from][to] == -1) return QVector<int>();
    QVector<int> path;
    int current = from;
    while (current != to) {
        path.append(current);
        current = result.next[current][to];
        if (current == -1) return QVector<int>();
    }
    path.append(to);
    return path;
}

/** @brief 检测负权环 */
bool FloydWarshall::hasNegativeCycle(
    const QVector<QVector<double>>& adjacency)
{
    auto result = allPairsShortestPaths(adjacency);
    return result.hasNegativeCycle;
}

/** @brief 重置统计 */
void FloydWarshall::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
