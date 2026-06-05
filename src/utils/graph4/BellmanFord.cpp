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
    , m_vertexCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 添加有向边
 *  @param from 起点 @param to 终点 @param weight 权重 */
void BellmanFord::addEdge(int from, int to, double weight)
{
    Edge e;
    e.from = from;
    e.to = to;
    e.weight = weight;
    m_edges.append(e);

    /* 自动扩展顶点数 */
    int maxVertex = qMax(from, to) + 1;
    if (maxVertex > m_vertexCount) {
        m_vertexCount = maxVertex;
    }
}

/** @brief 计算最短路径
 *  @param source 起点 @param target 终点
 *  @return 路径节点序列 */
QVector<int> BellmanFord::shortestPath(int source, int target)
{
    QElapsedTimer timer;
    timer.start();

    int V = m_vertexCount;
    if (V == 0 || source < 0 || source >= V || target < 0 || target >= V) {
        return QVector<int>();
    }

    const double INF = 1e18;
    QVector<double> dist(V, INF);
    QVector<int> pred(V, -1);
    dist[source] = 0.0;

    /* 松弛V-1次 */
    for (int iter = 0; iter < V - 1; ++iter) {
        bool updated = false;
        for (const auto& e : m_edges) {
            if (dist[e.from] < INF &&
                dist[e.from] + e.weight < dist[e.to]) {
                dist[e.to] = dist[e.from] + e.weight;
                pred[e.to] = e.from;
                updated = true;
            }
        }
        if (!updated) break;
    }

    /* 检测负权环 */
    bool negCycle = false;
    for (const auto& e : m_edges) {
        if (dist[e.from] < INF &&
            dist[e.from] + e.weight < dist[e.to]) {
            negCycle = true;
            break;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalQueries;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    /* 负权环或不可达时返回空 */
    if (negCycle || dist[target] >= INF) {
        emit queryCompleted(source, target, 0);
        return QVector<int>();
    }

    /* 回溯路径 */
    QVector<int> path;
    int current = target;
    while (current != -1) {
        path.prepend(current);
        current = pred[current];
    }

    emit queryCompleted(source, target, path.size());
    return path;
}

/** @brief 检测负权环 @return 是否存在负权环 */
bool BellmanFord::hasNegativeCycle()
{
    int V = m_vertexCount;
    if (V == 0) return false;

    /* 从虚拟源点(距离全0)执行V次松弛 */
    QVector<double> dist(V, 0.0);

    for (int iter = 0; iter < V; ++iter) {
        bool updated = false;
        for (const auto& e : m_edges) {
            if (dist[e.from] + e.weight < dist[e.to]) {
                dist[e.to] = dist[e.from] + e.weight;
                updated = true;
            }
        }
        if (!updated) return false;
    }
    return true;
}

/** @brief 设置顶点数 @param n 顶点数 */
void BellmanFord::setVertexCount(int n)
{
    m_vertexCount = qMax(0, n);
}

/** @brief 重置统计 */
void BellmanFord::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
