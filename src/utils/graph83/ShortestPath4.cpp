/**
 * @file ShortestPath4.cpp
 * @brief 最短路径算法实现
 *
 * 实现Dijkstra、Bellman-Ford和Floyd-Warshall三种经典最短路径算法，
 * 支持有向/无向加权图，包含负环检测。
 */

#include "utils/graph83/ShortestPath4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
ShortestPath4::ShortestPath4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param n 顶点数
 */
void ShortestPath4::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
    m_negCycle = false;
}

/**
 * @brief 添加带权边
 * @param u 起点
 * @param v 终点
 * @param weight 边权重
 * @param directed 是否为有向边
 */
void ShortestPath4::addEdge(int u, int v, double weight, bool directed)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append({v, weight});
    if (!directed) {
        m_adj[v].append({u, weight});
    }
}

/**
 * @brief Dijkstra单源最短路径（非负权重）
 * @param source 源顶点
 * @return 从源到各顶点的最短距离
 */
QVector<double> ShortestPath4::dijkstra(int source)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> dist(m_n, 1e18);
    if (source < 0 || source >= m_n) return dist;

    dist[source] = 0.0;

    // 优先队列：(距离, 顶点)
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>> pq;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue; // 跳过过时条目

        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double w = edge.second;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    int reachable = 0;
    for (double d : dist) { if (d < 1e18) reachable++; }
    emit solveCompleted(source, reachable);
    return dist;
}

/**
 * @brief Bellman-Ford单源最短路径（支持负权重）
 * @param source 源顶点
 * @return 从源到各顶点的最短距离
 */
QVector<double> ShortestPath4::bellmanFord(int source)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> dist(m_n, 1e18);
    if (source < 0 || source >= m_n) return dist;

    dist[source] = 0.0;
    m_negCycle = false;

    // 收集所有边
    struct Edge { int u, v; double w; };
    QVector<Edge> edges;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& e : m_adj[u]) {
            edges.append({u, e.first, e.second});
        }
    }

    // 松弛操作：V-1轮
    for (int i = 0; i < m_n - 1; ++i) {
        bool updated = false;
        for (const auto& e : edges) {
            if (dist[e.u] < 1e18 && dist[e.u] + e.w < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.w;
                updated = true;
            }
        }
        if (!updated) break; // 提前终止
    }

    // 检测负环：第V轮如果能继续松弛说明存在负环
    for (const auto& e : edges) {
        if (dist[e.u] < 1e18 && dist[e.u] + e.w < dist[e.v]) {
            m_negCycle = true;
            break;
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    int reachable = 0;
    for (double d : dist) { if (d < 1e18) reachable++; }
    emit solveCompleted(source, reachable);
    return dist;
}

/**
 * @brief Floyd-Warshall全源最短路径
 * @return 距离矩阵，dist[i][j]为i到j的最短距离
 */
QVector<QVector<double>> ShortestPath4::floydWarshall()
{
    QElapsedTimer timer;
    timer.start();

    // 初始化距离矩阵
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, 1e18));
    for (int i = 0; i < m_n; ++i) dist[i][i] = 0.0;

    // 填入边权重
    for (int u = 0; u < m_n; ++u) {
        for (const auto& e : m_adj[u]) {
            dist[u][e.first] = qMin(dist[u][e.first], e.second);
        }
    }

    // Floyd-Warshall三重循环
    for (int k = 0; k < m_n; ++k) {
        for (int i = 0; i < m_n; ++i) {
            for (int j = 0; j < m_n; ++j) {
                if (dist[i][k] < 1e18 && dist[k][j] < 1e18) {
                    dist[i][j] = qMin(dist[i][j], dist[i][k] + dist[k][j]);
                }
            }
        }
    }

    // 检测负环（对角线元素<0）
    m_negCycle = false;
    for (int i = 0; i < m_n; ++i) {
        if (dist[i][i] < 0.0) { m_negCycle = true; break; }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    return dist;
}

/**
 * @brief 重置统计信息
 */
void ShortestPath4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
