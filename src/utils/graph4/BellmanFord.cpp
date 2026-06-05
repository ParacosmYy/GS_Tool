/**
 * @file BellmanFord.cpp
 * @brief Bellman-Ford最短路径实现
 */

#include "utils/graph4/BellmanFord.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
BellmanFord::BellmanFord(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算单源最短路径 */
BellmanFord::PathResult BellmanFord::shortestPaths(
    int vertexCount, const QVector<Edge>& edges, int source)
{
    QElapsedTimer timer;
    timer.start();

    PathResult result;
    result.distances.resize(vertexCount);
    result.predecessors.resize(vertexCount);

    const double INF = 1e18;
    for (int i = 0; i < vertexCount; ++i) {
        result.distances[i] = INF;
        result.predecessors[i] = -1;
    }
    result.distances[source] = 0.0;

    /* 松弛V-1次 */
    for (int iter = 0; iter < vertexCount - 1; ++iter) {
        bool updated = false;
        for (const auto& e : edges) {
            if (result.distances[e.from] < INF &&
                result.distances[e.from] + e.weight < result.distances[e.to]) {
                result.distances[e.to] = result.distances[e.from] + e.weight;
                result.predecessors[e.to] = e.from;
                updated = true;
            }
        }
        if (!updated) break;
    }

    /* 检测负权环 */
    result.hasNegativeCycle = false;
    for (const auto& e : edges) {
        if (result.distances[e.from] < INF &&
            result.distances[e.from] + e.weight < result.distances[e.to]) {
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

    emit queryCompleted(source, result.hasNegativeCycle);
    return result;
}

/** @brief 检测负权环 */
bool BellmanFord::hasNegativeCycle(int vertexCount,
                                    const QVector<Edge>& edges)
{
    /* 从虚拟源点(距离全0)执行V次松弛 */
    QVector<double> dist(vertexCount, 0.0);

    for (int iter = 0; iter < vertexCount; ++iter) {
        bool updated = false;
        for (const auto& e : edges) {
            if (dist[e.from] + e.weight < dist[e.to]) {
                dist[e.to] = dist[e.from] + e.weight;
                updated = true;
            }
        }
        if (!updated) return false;
    }
    return true;
}

/** @brief 回溯最短路径 */
QVector<int> BellmanFord::reconstructPath(const PathResult& result,
                                           int target) const
{
    QVector<int> path;
    int current = target;
    while (current != -1) {
        path.prepend(current);
        current = result.predecessors[current];
    }
    return path;
}

/** @brief 重置统计 */
void BellmanFord::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
