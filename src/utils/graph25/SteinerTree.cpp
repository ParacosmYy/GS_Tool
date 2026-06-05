/**
 * @file SteinerTree.cpp
 * @brief Steiner树近似算法实现 — 基于度量闭包
 */

#include "utils/graph25/SteinerTree.h"

#include <QElapsedTimer>

#include <cmath>
#include <limits>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SteinerTree::SteinerTree(QObject* parent)
    : QObject(parent)
    , m_vertexCount(0)
{
}

/** @brief 构建带权无向图 @param edges 边列表 @param vertexCount 顶点数 */
void SteinerTree::buildGraph(const QList<Edge>& edges, int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    m_vertexCount = vertexCount;
    m_edges = edges;

    /* 初始化无穷大的邻接矩阵 */
    const double INF = std::numeric_limits<double>::infinity();
    m_adjMatrix.assign(vertexCount, QVector<double>(vertexCount, INF));
    for (int i = 0; i < vertexCount; ++i) {
        m_adjMatrix[i][i] = 0.0;
    }

    /* 填充边权重(无向图: 对称) */
    for (const auto& e : edges) {
        if (e.u >= 0 && e.u < vertexCount && e.v >= 0 && e.v < vertexCount) {
            m_adjMatrix[e.u][e.v] = std::min(m_adjMatrix[e.u][e.v], e.weight);
            m_adjMatrix[e.v][e.u] = std::min(m_adjMatrix[e.v][e.u], e.weight);
        }
    }

    ++m_stats.totalClosuresBuilt;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalClosuresBuilt + m_stats.totalTreesComputed + m_stats.totalMstComputed);
}

/** @brief 添加一条带权无向边 @param u 起点 @param v 终点 @param weight 权重 */
void SteinerTree::addEdge(int u, int v, double weight)
{
    int maxIdx = std::max(u, v) + 1;
    if (maxIdx > m_vertexCount) {
        const double INF = std::numeric_limits<double>::infinity();
        for (auto& row : m_adjMatrix) {
            row.resize(maxIdx, INF);
        }
        m_adjMatrix.resize(maxIdx, QVector<double>(maxIdx, INF));
        for (int i = m_vertexCount; i < maxIdx; ++i) {
            m_adjMatrix[i][i] = 0.0;
        }
        m_vertexCount = maxIdx;
    }
    const double cur = m_adjMatrix[u][v];
    m_adjMatrix[u][v] = std::min(cur, weight);
    m_adjMatrix[v][u] = std::min(m_adjMatrix[v][u], weight);
    m_edges.append({u, v, weight});
}

/** @brief Floyd-Warshall全源最短路 @return 距离矩阵 */
QVector<QVector<double>> SteinerTree::floydWarshall() const
{
    int n = m_vertexCount;
    QVector<QVector<double>> dist = m_adjMatrix;
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
    return dist;
}

/** @brief Floyd-Warshall同时返回前驱矩阵 @param dist 输出距离矩阵 @param next 输出前驱矩阵 */
void SteinerTree::floydWithPath(QVector<QVector<double>>& dist,
                                 QVector<QVector<int>>& next) const
{
    int n = m_vertexCount;
    const double INF = std::numeric_limits<double>::infinity();
    dist = m_adjMatrix;
    next.assign(n, QVector<int>(n, -1));

    /* 初始化前驱矩阵 */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && dist[i][j] < INF) {
                next[i][j] = j;
            }
        }
    }

    /* 三重循环松弛 */
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }
}

/** @brief 构建度量闭包图 @param terminals 终端集合 @return 完全图边列表 */
QList<SteinerTree::Edge> SteinerTree::buildMetricClosure(const QSet<int>& terminals)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> dist;
    QVector<QVector<int>> next;
    floydWithPath(dist, next);

    QList<Edge> closure;
    QList<int> termList = terminals.values();
    std::sort(termList.begin(), termList.end());

    /* 完全图: 所有terminal对之间的最短路径边 */
    for (int i = 0; i < termList.size(); ++i) {
        for (int j = i + 1; j < termList.size(); ++j) {
            int u = termList[i], v = termList[j];
            if (dist[u][v] < std::numeric_limits<double>::infinity()) {
                closure.append({u, v, dist[u][v]});
            }
        }
    }

    ++m_stats.totalClosuresBuilt;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalClosuresBuilt + m_stats.totalTreesComputed + m_stats.totalMstComputed);
    return closure;
}

/** @brief Prim算法求MST @param vertices 顶点列表 @param distMatrix 距离矩阵 @return MST边列表 */
QList<SteinerTree::Edge> SteinerTree::primMST(const QVector<int>& vertices,
                                               const QVector<QVector<double>>& distMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = vertices.size();
    if (n <= 1) return {};

    QList<Edge> mstEdges;
    const double INF = std::numeric_limits<double>::infinity();
    QVector<double> minCost(n, INF);
    QVector<int> parent(n, -1);
    QVector<bool> inMST(n, false);

    minCost[0] = 0.0;

    for (int iter = 0; iter < n; ++iter) {
        /* 选取最小代价顶点 */
        int u = -1;
        double best = INF;
        for (int j = 0; j < n; ++j) {
            if (!inMST[j] && minCost[j] < best) {
                best = minCost[j];
                u = j;
            }
        }
        if (u == -1) break;
        inMST[u] = true;

        if (parent[u] != -1) {
            mstEdges.append({vertices[parent[u]], vertices[u], best});
        }

        /* 更新邻接代价 */
        for (int v = 0; v < n; ++v) {
            if (!inMST[v]) {
                double d = distMatrix[vertices[u]][vertices[v]];
                if (d < minCost[v]) {
                    minCost[v] = d;
                    parent[v] = u;
                }
            }
        }
    }

    ++m_stats.totalMstComputed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalClosuresBuilt + m_stats.totalTreesComputed + m_stats.totalMstComputed);
    return mstEdges;
}

/** @brief 将度量闭包MST边映射回原图路径 @param closureEdges 闭包MST边 @param next 前驱矩阵 @return 原图路径边集合 */
QList<SteinerTree::Edge> SteinerTree::mapClosureToOriginal(
    const QList<Edge>& closureEdges,
    const QVector<QVector<int>>& next)
{
    QList<Edge> result;
    const double INF = std::numeric_limits<double>::infinity();

    for (const auto& ce : closureEdges) {
        int cur = ce.u;
        while (cur != ce.v && next[cur][ce.v] != -1) {
            int nxt = next[cur][ce.v];
            double w = (m_adjMatrix[cur][nxt] < INF) ? m_adjMatrix[cur][nxt] : 0.0;
            result.append({cur, nxt, w});
            cur = nxt;
        }
    }

    /* 去重(无向边 u-v 与 v-u 视为相同) */
    QSet<QPair<int, int>> seen;
    QList<Edge> unique;
    for (const auto& e : result) {
        int lo = std::min(e.u, e.v), hi = std::max(e.u, e.v);
        auto key = qMakePair(lo, hi);
        if (!seen.contains(key)) {
            seen.insert(key);
            unique.append(e);
        }
    }
    return unique;
}

/** @brief 计算Steiner树(2-近似) @param terminals 终端集合 @return 树边列表 */
QList<SteinerTree::Edge> SteinerTree::computeSteinerTree(const QSet<int>& terminals)
{
    QElapsedTimer timer;
    timer.start();

    if (terminals.size() <= 1) return {};
    if (terminals.size() == 2) {
        auto it = terminals.begin();
        int u = *it; ++it; int v = *it;
        auto dist = floydWarshall();
        QList<Edge> path;
        int cur = u;
        while (cur != v) {
            int nxt = -1;
            double best = dist[cur][v];
            for (int j = 0; j < m_vertexCount; ++j) {
                if (j != cur && m_adjMatrix[cur][j] < std::numeric_limits<double>::infinity()) {
                    if (std::abs(m_adjMatrix[cur][j] + dist[j][v] - dist[cur][v]) < 1e-9) {
                        nxt = j;
                        break;
                    }
                }
            }
            if (nxt == -1) break;
            path.append({cur, nxt, m_adjMatrix[cur][nxt]});
            cur = nxt;
        }
        ++m_stats.totalTreesComputed;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum
            / (m_stats.totalClosuresBuilt + m_stats.totalTreesComputed + m_stats.totalMstComputed);
        return path;
    }

    /* 步骤1: 计算全源最短路和前驱矩阵 */
    QVector<QVector<double>> dist;
    QVector<QVector<int>> next;
    floydWithPath(dist, next);

    /* 步骤2: 构建度量闭包 */
    QList<int> termList = terminals.values();
    std::sort(termList.begin(), termList.end());

    QList<Edge> closureEdges;
    for (int i = 0; i < termList.size(); ++i) {
        for (int j = i + 1; j < termList.size(); ++j) {
            int u = termList[i], v = termList[j];
            if (dist[u][v] < std::numeric_limits<double>::infinity()) {
                closureEdges.append({u, v, dist[u][v]});
            }
        }
    }

    /* 步骤3: 在度量闭包上求MST */
    auto mstEdges = primMST(termList, dist);

    /* 步骤4: 映射回原图路径 */
    auto treeEdges = mapClosureToOriginal(mstEdges, next);

    /* 计算总权重 */
    double totalWeight = 0.0;
    for (const auto& e : treeEdges) totalWeight += e.weight;

    ++m_stats.totalTreesComputed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalClosuresBuilt + m_stats.totalTreesComputed + m_stats.totalMstComputed);

    emit treeComputed(treeEdges.size(), totalWeight);
    return treeEdges;
}

/** @brief 清空图 */
void SteinerTree::clear()
{
    m_adjMatrix.clear();
    m_edges.clear();
    m_vertexCount = 0;
}

/** @brief 重置统计 */
void SteinerTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
