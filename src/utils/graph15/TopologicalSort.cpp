/**
 * @file TopologicalSort.cpp
 * @brief 拓扑排序实现 — Kahn算法与DFS双模式
 */

#include "TopologicalSort.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QStack>
#include <algorithm>

/* ---------- 构造函数 ---------- */

TopologicalSort::TopologicalSort(QObject* parent)
    : QObject(parent)
{
}

/* ---------- Kahn算法(BFS) ---------- */

TopologicalSort::SortResult TopologicalSort::sortKahn(
    int numVertices, const EdgeList& edges) const
{
    QElapsedTimer timer;
    timer.start();

    SortResult result;
    if (numVertices <= 0) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
            ? m_timeSum / m_stats.totalSorts : 0.0;
        return result;
    }

    /* 构建邻接表和入度数组 */
    auto adj = buildAdjList(numVertices, edges);
    QVector<int> inDegree(numVertices, 0);

    for (const auto& edge : edges) {
        inDegree[edge.second]++;
    }

    /* 初始化队列: 所有入度为0的顶点 */
    QQueue<int> queue;
    for (int i = 0; i < numVertices; ++i) {
        if (inDegree[i] == 0) {
            queue.enqueue(i);
        }
    }

    /* BFS遍历，按层级处理 */
    while (!queue.isEmpty()) {
        QVector<int> currentLevel;
        int levelSize = queue.size();

        for (int i = 0; i < levelSize; ++i) {
            int node = queue.dequeue();
            result.order.append(node);
            currentLevel.append(node);

            /* 减少邻居入度 */
            for (int neighbor : adj[node]) {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0) {
                    queue.enqueue(neighbor);
                }
            }
        }

        if (!currentLevel.isEmpty()) {
            result.levels.append(currentLevel);
        }
    }

    /* 检测环: 若排序结果不足则存在环 */
    if (result.order.size() < numVertices) {
        result.hasCycle = true;
        m_stats.totalCyclesDetected++;

        /* 收集环中节点: 入度仍>0的节点 */
        QSet<int> visitedSet(result.order.begin(), result.order.end());
        for (int i = 0; i < numVertices; ++i) {
            if (!visitedSet.contains(i)) {
                result.cycleNodes.append(i);
            }
        }
    }

    m_stats.totalSorts++;
    m_stats.totalVertices += numVertices;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
        ? m_timeSum / m_stats.totalSorts : 0.0;

    emit sortCompleted(numVertices, result.hasCycle);
    return result;
}

/* ---------- DFS拓扑排序 ---------- */

TopologicalSort::SortResult TopologicalSort::sortDFS(
    int numVertices, const EdgeList& edges) const
{
    QElapsedTimer timer;
    timer.start();

    SortResult result;
    if (numVertices <= 0) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
            ? m_timeSum / m_stats.totalSorts : 0.0;
        return result;
    }

    auto adj = buildAdjList(numVertices, edges);

    /* state: 0=未访问, 1=访问中, 2=已完成 */
    QMap<int, int> state;
    QVector<int> order;

    for (int i = 0; i < numVertices; ++i) {
        state[i] = 0;
    }

    for (int i = 0; i < numVertices; ++i) {
        if (state[i] == 0) {
            if (dfsVisit(i, adj, state, order, result.cycleNodes)) {
                result.hasCycle = true;
                m_stats.totalCyclesDetected++;
            }
        }
    }

    /* DFS后序逆序即为拓扑序 */
    std::reverse(order.begin(), order.end());
    result.order = order;

    m_stats.totalSorts++;
    m_stats.totalVertices += numVertices;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
        ? m_timeSum / m_stats.totalSorts : 0.0;

    emit sortCompleted(numVertices, result.hasCycle);
    return result;
}

/* ---------- 环检测 ---------- */

bool TopologicalSort::hasCycle(int numVertices, const EdgeList& edges) const
{
    auto result = sortKahn(numVertices, edges);
    return result.hasCycle;
}

/* ---------- 统计 ---------- */

TopologicalSort::Stats TopologicalSort::stats() const { return m_stats; }

void TopologicalSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: 构建邻接表 ---------- */

QMap<int, QVector<int>> TopologicalSort::buildAdjList(
    int numVertices, const EdgeList& edges) const
{
    QMap<int, QVector<int>> adj;
    for (int i = 0; i < numVertices; ++i) {
        adj[i] = QVector<int>();
    }

    for (const auto& edge : edges) {
        adj[edge.first].append(edge.second);
    }
    return adj;
}

/* ---------- 私有: DFS递归 ---------- */

bool TopologicalSort::dfsVisit(int node,
                                const QMap<int, QVector<int>>& adj,
                                QMap<int, int>& state,
                                QVector<int>& order,
                                QVector<int>& cycleNodes) const
{
    state[node] = 1; /* 访问中 */

    for (int neighbor : adj[node]) {
        if (state[neighbor] == 1) {
            /* 回边: 检测到环 */
            cycleNodes.append(neighbor);
            cycleNodes.append(node);
            return true;
        }
        if (state[neighbor] == 0) {
            if (dfsVisit(neighbor, adj, state, order, cycleNodes)) {
                return true;
            }
        }
    }

    state[node] = 2; /* 已完成 */
    order.append(node);
    return false;
}
