/**
 * @file PrimMST.cpp
 * @brief Prim最小生成树实现
 */

#include "utils/mst2/PrimMST.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
PrimMST::PrimMST(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 从邻接矩阵计算MST */
PrimMST::MstResult PrimMST::computeFromMatrix(
    const QVector<QVector<double>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    MstResult result;
    int n = adjacency.size();
    if (n == 0) return result;

    const double INF = 1e18;
    QVector<double> key(n, INF);
    QVector<int> parent(n, -1);
    QVector<bool> inMST(n, false);

    key[0] = 0.0;

    for (int iter = 0; iter < n; ++iter) {
        /* 找最小key的未加入顶点 */
        int u = -1;
        double minKey = INF;
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }
        }

        if (u == -1) break;
        inMST[u] = true;

        for (int v = 0; v < n; ++v) {
            double w = adjacency[u][v];
            if (w >= 0 && !inMST[v] && w < key[v]) {
                key[v] = w;
                parent[v] = u;
            }
        }
    }

    /* 构建结果 */
    result.totalWeight = 0.0;
    for (int i = 1; i < n; ++i) {
        if (parent[i] != -1) {
            result.edges.append({parent[i], i});
            double w = adjacency[parent[i]][i];
            result.weights.append(w);
            result.totalWeight += w;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    emit mstCompleted(result.edges.size(), result.totalWeight);
    return result;
}

/** @brief 从邻接表计算MST */
PrimMST::MstResult PrimMST::computeFromList(
    int vertexCount,
    const QVector<QVector<AdjEdge>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    MstResult result;
    int n = vertexCount;
    const double INF = 1e18;

    QVector<double> key(n, INF);
    QVector<int> parent(n, -1);
    QVector<bool> inMST(n, false);

    key[0] = 0.0;

    for (int iter = 0; iter < n; ++iter) {
        int u = -1;
        double minKey = INF;
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }
        }

        if (u == -1) break;
        inMST[u] = true;

        for (const auto& e : adjList[u]) {
            if (!inMST[e.to] && e.weight < key[e.to]) {
                key[e.to] = e.weight;
                parent[e.to] = u;
            }
        }
    }

    result.totalWeight = 0.0;
    for (int i = 1; i < n; ++i) {
        if (parent[i] != -1) {
            result.edges.append({parent[i], i});
            result.totalWeight += key[i];
            result.weights.append(key[i]);
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    emit mstCompleted(result.edges.size(), result.totalWeight);
    return result;
}

/** @brief 重置统计 */
void PrimMST::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
