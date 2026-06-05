#include "ShortestPath5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class ShortestPath5
 * @brief 最短路径求解器实现
 *
 * 提供Dijkstra和Bellman-Ford两种经典最短路径算法:
 *
 * Dijkstra: 适用于非负权图，使用优先队列(最小堆)优化。
 * 时间复杂度O((V+E) log V)。贪心策略，每次选取当前最短路径的顶点。
 *
 * Bellman-Ford: 适用于含负权边的图，可检测负权环。
 * 时间复杂度O(V*E)。对所有边进行V-1轮松弛操作。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ShortestPath5::ShortestPath5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Dijkstra单源最短路径算法
 *
 * 从源点出发，使用最小距离优先策略逐步扩展最短路径树。
 * 每次从优先队列中取出当前距离最小的顶点u，
 * 对u的所有出边进行松弛操作: 如果dist[u] + w(u,v) < dist[v]，更新dist[v]。
 *
 * @param adjList 邻接表，adjList[u]包含{v, weight}对
 * @param source 源点索引
 * @return 从source到各顶点的最短距离(infinity表示不可达)
 */
QVector<double> ShortestPath5::dijkstra(const QVector<QVector<QPair<int, double>>>& adjList, int source)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjList.size();
    const double INF = 1e18;
    QVector<double> dist(n, INF);

    if (n == 0 || source < 0 || source >= n) {
        m_timeSum += timer.elapsed();
        return dist;
    }

    dist[source] = 0.0;

    /* 优先队列: {距离, 顶点}，最小堆 */
    QVector<QPair<double, int>> pq;
    pq.append({0.0, source});

    /* 已确定最短路径的标记 */
    QVector<bool> visited(n, false);

    while (!pq.isEmpty()) {
        /* 取出最小距离顶点 */
        int minIdx = 0;
        for (int i = 1; i < pq.size(); ++i) {
            if (pq[i].first < pq[minIdx].first) {
                minIdx = i;
            }
        }

        double d = pq[minIdx].first;
        int u = pq[minIdx].second;
        pq.removeAt(minIdx);

        if (visited[u]) continue;
        visited[u] = true;

        /* 松弛操作 */
        for (const auto& edge : adjList[u]) {
            int v = edge.first;
            double w = edge.second;
            if (w < 0) continue; /* Dijkstra不支持负权 */

            m_stats.totalRelaxations++;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.append({dist[v], v});
            }
        }
    }

    m_stats.totalPathsComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPathsComputed);

    emit pathComputed(source, n, false);

    return dist;
}

/**
 * @brief Bellman-Ford单源最短路径算法
 *
 * 对所有边执行V-1轮松弛操作，可处理负权边。
 * 第V轮检查是否还能松弛，若能则存在负权环。
 *
 * 松弛操作: 对每条边(u,v,w)，如果dist[u] + w < dist[v]，更新dist[v]。
 *
 * @param adjList 邻接表
 * @param source 源点索引
 * @param nodeCount 图中顶点数
 * @return 从source到各顶点的最短距离(infinity表示不可达，负无穷表示负环)
 */
QVector<double> ShortestPath5::bellmanFord(
    const QVector<QVector<QPair<int, double>>>& adjList, int source, int nodeCount)
{
    QElapsedTimer timer;
    timer.start();

    const double INF = 1e18;
    const double NEG_INF = -1e18;
    QVector<double> dist(nodeCount, INF);

    if (nodeCount == 0 || source < 0 || source >= nodeCount) {
        m_timeSum += timer.elapsed();
        return dist;
    }

    dist[source] = 0.0;

    /* V-1轮松弛 */
    for (int iter = 0; iter < nodeCount - 1; ++iter) {
        bool updated = false;
        for (int u = 0; u < adjList.size() && u < nodeCount; ++u) {
            if (dist[u] >= INF) continue;
            for (const auto& edge : adjList[u]) {
                int v = edge.first;
                double w = edge.second;
                if (v < 0 || v >= nodeCount) continue;

                m_stats.totalRelaxations++;
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    updated = true;
                }
            }
        }
        if (!updated) break;
    }

    /* 检测负权环: 第V轮松弛 */
    bool hasNegativeCycle = false;
    for (int u = 0; u < adjList.size() && u < nodeCount; ++u) {
        if (dist[u] >= INF) continue;
        for (const auto& edge : adjList[u]) {
            int v = edge.first;
            double w = edge.second;
            if (v < 0 || v >= nodeCount) continue;
            if (dist[u] + w < dist[v]) {
                hasNegativeCycle = true;
                dist[v] = NEG_INF;
            }
        }
    }

    m_stats.totalPathsComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPathsComputed);

    emit pathComputed(source, nodeCount, hasNegativeCycle);

    return dist;
}

/**
 * @brief 重置所有统计数据
 *
 * 将路径计算计数、松弛操作计数和计时归零。
 */
void ShortestPath5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
