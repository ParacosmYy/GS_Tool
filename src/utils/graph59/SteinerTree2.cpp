/**
 * @file SteinerTree2.cpp
 * @brief Steiner树2实现 — SPH近似+度量闭包
 *
 * 实现基于最短路径启发式(SPH)的Steiner树近似算法。
 * 通过构建终端集的度量闭包，贪心地扩展树以连接所有终端。
 */

#include "utils/graph59/SteinerTree2.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SteinerTree2::SteinerTree2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的邻接关系
 * @param n 顶点数
 * @param edges 边列表，每条边为 ((u,v), weight)
 */
void SteinerTree2::setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    for (auto& adj : m_adj) adj.clear();

    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double w = edge.second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            m_adj[u].append({v, w});
            m_adj[v].append({u, w});
        }
    }
}

/**
 * @brief 设置终端节点集合
 * @param terminals Steiner终端节点索引列表
 */
void SteinerTree2::setTerminals(const QVector<int>& terminals)
{
    m_terminals = terminals;
}

/**
 * @brief 求解Steiner树
 * @return Steiner树的总代价
 *
 * 使用SPH(Shortest Path Heuristic)算法:
 * 1. 从第一个终端开始
 * 2. 每步找距离当前树最近的未连接终端
 * 3. 将最短路径加入树中
 * 4. 重复直到所有终端连通
 */
double SteinerTree2::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_treeEdges.clear();
    m_cost = 0.0;

    if (m_terminals.isEmpty() || m_n == 0) {
        return 0.0;
    }

    const int t = m_terminals.size();
    if (t == 1) {
        return 0.0;
    }

    /* ---- 度量闭包: 终端间最短距离矩阵 ---- */
    QVector<QVector<double>> dist(t, QVector<double>(t, 1e18));
    QVector<QVector<int>> pred(t, QVector<int>(t, -1));

    for (int i = 0; i < t; ++i) {
        int src = m_terminals[i];
        QVector<double> d = dijkstra(src);

        for (int j = 0; j < t; ++j) {
            int dst = m_terminals[j];
            dist[i][j] = d[dst];
        }
    }

    /* ---- SPH: 贪心扩展Steiner树 ---- */
    QVector<bool> inTree(t, false);
    inTree[0] = true;
    int inTreeCount = 1;

    /* 树中顶点集合（包括Steiner点） */
    QSet<int> treeVertices;
    treeVertices.insert(m_terminals[0]);

    /* 记录已加入的路径边 */
    QVector<QPair<int,int>> resultEdges;
    double totalCost = 0.0;

    while (inTreeCount < t) {
        /* 找距离当前树最近的未连接终端 */
        double bestDist = 1e18;
        int bestTerminal = -1;
        int bestTreeTerminal = -1;

        for (int i = 0; i < t; ++i) {
            if (!inTree[i]) {
                for (int j = 0; j < t; ++j) {
                    if (inTree[j] && dist[j][i] < bestDist) {
                        bestDist = dist[j][i];
                        bestTerminal = i;
                        bestTreeTerminal = j;
                    }
                }
            }
        }

        if (bestTerminal < 0) break;

        /* ---- 回溯最短路径获取边 ---- */
        /* 从m_terminals[bestTreeTerminal]到m_terminals[bestTerminal] */
        QVector<double> dFromTree = dijkstra(m_terminals[bestTreeTerminal]);
        QVector<double> dToTarget = dijkstra(m_terminals[bestTerminal]);

        /* 通过中间顶点重建路径 */
        int src = m_terminals[bestTreeTerminal];
        int dst = m_terminals[bestTerminal];

        /* 使用BFS回溯找路径 */
        QVector<int> prev(m_n, -1);
        QVector<double> distMap(m_n, 1e18);
        distMap[src] = 0.0;

        /* Dijkstra with predecessor tracking */
        QVector<bool> visited(m_n, false);
        for (int iter = 0; iter < m_n; ++iter) {
            int u = -1;
            double minD = 1e18;
            for (int v = 0; v < m_n; ++v) {
                if (!visited[v] && distMap[v] < minD) {
                    minD = distMap[v];
                    u = v;
                }
            }
            if (u < 0 || u == dst) break;
            visited[u] = true;

            for (const auto& edge : m_adj[u]) {
                double newDist = distMap[u] + edge.second;
                if (newDist < distMap[edge.first]) {
                    distMap[edge.first] = newDist;
                    prev[edge.first] = u;
                }
            }
        }

        /* 回溯路径收集边 */
        int cur = dst;
        while (cur != src && cur >= 0 && prev[cur] >= 0) {
            int p = prev[cur];
            resultEdges.append({p, cur});
            treeVertices.insert(p);
            treeVertices.insert(cur);
            cur = p;
        }

        totalCost += bestDist;
        inTree[bestTerminal] = true;
        inTreeCount++;
    }

    m_treeEdges = resultEdges;
    m_cost = totalCost;

    /* 统计Steiner节点数（非终端的树节点） */
    int steinerCount = 0;
    for (int v : treeVertices) {
        bool isTerminal = false;
        for (int t : m_terminals) {
            if (t == v) { isTerminal = true; break; }
        }
        if (!isTerminal) steinerCount++;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_stats.totalSteinerNodes += steinerCount;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(m_cost, m_treeEdges.size(), steinerCount);
    return m_cost;
}

/**
 * @brief Dijkstra单源最短路径
 * @param src 源顶点
 * @return 从src到所有顶点的最短距离数组
 */
QVector<double> SteinerTree2::dijkstra(int src) const
{
    QVector<double> dist(m_n, 1e18);
    QVector<bool> visited(m_n, false);
    dist[src] = 0.0;

    for (int iter = 0; iter < m_n; ++iter) {
        int u = -1;
        double minD = 1e18;
        for (int v = 0; v < m_n; ++v) {
            if (!visited[v] && dist[v] < minD) {
                minD = dist[v];
                u = v;
            }
        }
        if (u < 0) break;
        visited[u] = true;

        for (const auto& edge : m_adj[u]) {
            double newDist = dist[u] + edge.second;
            if (newDist < dist[edge.first]) {
                dist[edge.first] = newDist;
            }
        }
    }

    return dist;
}

/**
 * @brief 计算度量闭包（终端间的最短距离矩阵）
 * @return 终端间最短距离矩阵
 */
QVector<QVector<double>> SteinerTree2::metricClosure() const
{
    const int t = m_terminals.size();
    QVector<QVector<double>> closure(t, QVector<double>(t, 0.0));

    for (int i = 0; i < t; ++i) {
        QVector<double> d = dijkstra(m_terminals[i]);
        for (int j = 0; j < t; ++j) {
            closure[i][j] = d[m_terminals[j]];
        }
    }

    return closure;
}

/**
 * @brief 重置所有统计信息
 */
void SteinerTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
