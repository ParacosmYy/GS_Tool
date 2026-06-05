/**
 * @file BridgeDetector.cpp
 * @brief 桥边检测器(Tarjan)实现
 *
 * 实现基于DFS的Tarjan桥边/割点检测算法：
 * 维护disc[]和low[]数组，通过回边更新low值。
 */

#include "utils/graph174/BridgeDetector.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BridgeDetector::BridgeDetector(QObject* parent)
    : QObject(parent)
{
}

void BridgeDetector::buildGraph(const QVector<Edge>& edges, int vertexCount)
{
    m_adj.clear();
    m_adj.resize(vertexCount);
    for (const auto& e : edges) {
        addEdge(e.first, e.second);
    }
}

void BridgeDetector::addEdge(int u, int v)
{
    ensureSize(qMax(u, v) + 1);
    m_adj[u].append(v);
    m_adj[v].append(u);
}

void BridgeDetector::ensureSize(int vertex)
{
    if (vertex >= m_adj.size()) {
        m_adj.resize(vertex + 1);
    }
}

/**
 * @brief Tarjan DFS递归核心
 *
 * 对每个顶点u维护：
 * - disc[u]: DFS发现时间戳
 * - low[u]: u及其子树通过最多一条回边能到达的最早祖先
 *
 * 桥边条件: low[v] > disc[u]
 * 割点条件: low[v] >= disc[u] (u非根) 或 u是根且有两个DFS子节点
 */
void BridgeDetector::dfs(int u, int parent, QVector<int>& disc, QVector<int>& low,
                         QVector<bool>& visited, int& timer,
                         QVector<Edge>* bridges, QVector<int>* artPoints)
{
    visited[u] = true;
    disc[u] = low[u] = timer++;
    int children = 0;

    for (int v : m_adj[u]) {
        if (!visited[v]) {
            children++;
            dfs(v, u, disc, low, visited, timer, bridges, artPoints);
            low[u] = qMin(low[u], low[v]);

            /* 桥边检测 */
            if (bridges && low[v] > disc[u]) {
                bridges->append(qMakePair(qMin(u, v), qMax(u, v)));
            }

            /* 割点检测 */
            if (artPoints) {
                if (parent == -1) {
                    /* 根节点：DFS子树 >= 2 则为割点 */
                    if (children >= 2) {
                        artPoints->append(u);
                    }
                } else {
                    if (low[v] >= disc[u]) {
                        artPoints->append(u);
                    }
                }
            }
        } else if (v != parent) {
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/**
 * @brief 检测所有桥边
 */
QVector<BridgeDetector::Edge> BridgeDetector::findBridges()
{
    QVector<Edge> bridges;
    QVector<int> artPoints;
    findAll(bridges, artPoints);
    return bridges;
}

/**
 * @brief 检测所有割点
 */
QVector<int> BridgeDetector::findArticulationPoints()
{
    QVector<Edge> bridges;
    QVector<int> artPoints;
    findAll(bridges, artPoints);
    return artPoints;
}

/**
 * @brief 同时检测桥边和割点
 *
 * 执行一次DFS遍历，O(V+E)时间复杂度。
 */
void BridgeDetector::findAll(QVector<Edge>& bridges, QVector<int>& artPoints)
{
    QElapsedTimer timer;
    timer.start();

    bridges.clear();
    artPoints.clear();

    const int n = m_adj.size();
    if (n == 0) return;

    QVector<int> disc(n, -1);
    QVector<int> low(n, -1);
    QVector<bool> visited(n, false);
    int timeCounter = 0;

    /* 处理非连通图 */
    for (int i = 0; i < n; ++i) {
        if (!visited[i] && !m_adj[i].isEmpty()) {
            dfs(i, -1, disc, low, visited, timeCounter, &bridges, &artPoints);
        }
    }

    /* 去重割点 */
    std::sort(artPoints.begin(), artPoints.end());
    artPoints.erase(std::unique(artPoints.begin(), artPoints.end()), artPoints.end());

    /* 去重桥边 */
    std::sort(bridges.begin(), bridges.end());
    bridges.erase(std::unique(bridges.begin(), bridges.end()), bridges.end());

    /* 统计 */
    m_stats.totalSearches++;
    m_stats.bridgesFound += bridges.size();
    m_stats.articulationPointsFound += artPoints.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0)
        ? m_timeSum / m_stats.totalSearches : 0.0;

    emit detectionCompleted(bridges.size(), artPoints.size());
}

void BridgeDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
