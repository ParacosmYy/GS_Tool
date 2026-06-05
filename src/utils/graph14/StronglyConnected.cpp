/**
 * @file StronglyConnected.cpp
 * @brief 强连通分量算法实现 — Tarjan SCC + Kosaraju变体
 */

#include "utils/graph14/StronglyConnected.h"

#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
StronglyConnected::StronglyConnected(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置算法 @param algo 算法 */
void StronglyConnected::setAlgorithm(Algorithm algo)
{
    m_algorithm = algo;
}

/** @brief 查找SCC(邻接表) @param adjList 邻接表 @return SCC结果 */
StronglyConnected::SccResult StronglyConnected::findSCC(
    const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    SccResult result;
    int n = adjList.size();

    if (n == 0) return result;

    /* 统计边数 */
    int edgeCount = 0;
    for (const auto& neighbors : adjList) {
        edgeCount += neighbors.size();
    }

    result = (m_algorithm == Algorithm::Tarjan)
        ? tarjanSCC(adjList)
        : kosarajuSCC(adjList);

    /* 计算最大SCC */
    for (const auto& comp : result.components) {
        if (comp.size() > result.largestComponentSize) {
            result.largestComponentSize = static_cast<int>(comp.size());
        }
    }

    /* 构建缩点DAG */
    if (!result.components.isEmpty()) {
        int sccCount = static_cast<int>(result.components.size());
        result.componentId.resize(n);
        for (int i = 0; i < sccCount; ++i) {
            for (int v : result.components[i]) {
                result.componentId[v] = i;
            }
        }

        /* 构建DAG边 */
        QSet<QPair<int,int>> dagEdgeSet;
        for (int u = 0; u < n; ++u) {
            for (int v : adjList[u]) {
                int cu = result.componentId[u];
                int cv = result.componentId[v];
                if (cu != cv) {
                    dagEdgeSet.insert({cu, cv});
                }
            }
        }
        result.dagEdges = dagEdgeSet.values().toVector();
    }

    result.hasCycle = (result.components.size() < static_cast<size_t>(n));

    /* 统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalGraphsProcessed;
    m_stats.totalSccsFound += static_cast<int>(result.components.size());
    m_stats.totalVerticesProcessed += n;
    m_stats.totalEdgesProcessed += edgeCount;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGraphsProcessed;

    emit sccComplete(static_cast<int>(result.components.size()),
                     result.largestComponentSize);
    return result;
}

/** @brief 查找SCC(边列表) @param edges 边列表 @param vertexCount 顶点数 @return SCC结果 */
StronglyConnected::SccResult StronglyConnected::findSCCFromEdges(
    const QVector<QPair<int,int>>& edges, int vertexCount)
{
    /* 转换为邻接表 */
    QVector<QVector<int>> adjList(vertexCount);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < vertexCount) {
            adjList[e.first].append(e.second);
        }
    }
    return findSCC(adjList);
}

/** @brief 判断是否有环 @param adjList 邻接表 @return 有环返回true */
bool StronglyConnected::hasCycle(const QVector<QVector<int>>& adjList)
{
    if (adjList.isEmpty()) return false;

    SccResult result = findSCC(adjList);
    return result.hasCycle;
}

/** @brief 构建缩点DAG @param adjList 邻接表 @param componentId SCC编号 @param sccCount SCC数 @return DAG */
QVector<QVector<int>> StronglyConnected::buildCondensationDAG(
    const QVector<QVector<int>>& adjList,
    const QVector<int>& componentId, int sccCount)
{
    QVector<QVector<int>> dag(sccCount);
    QSet<QPair<int,int>> added;

    int n = adjList.size();
    for (int u = 0; u < n; ++u) {
        for (int v : adjList[u]) {
            int cu = componentId[u];
            int cv = componentId[v];
            if (cu != cv && !added.contains({cu, cv})) {
                dag[cu].append(cv);
                added.insert({cu, cv});
            }
        }
    }
    return dag;
}

/** @brief 拓扑排序 @param dagEdges DAG边 @param nodeCount 节点数 @return 拓扑序 */
QVector<int> StronglyConnected::topologicalSort(
    const QVector<QPair<int,int>>& dagEdges, int nodeCount)
{
    /* Kahn算法 */
    QVector<int> inDegree(nodeCount, 0);
    QVector<QVector<int>> adj(nodeCount);
    for (const auto& e : dagEdges) {
        adj[e.first].append(e.second);
        ++inDegree[e.second];
    }

    QStack<int> stack;
    for (int i = 0; i < nodeCount; ++i) {
        if (inDegree[i] == 0) stack.push(i);
    }

    QVector<int> order;
    order.reserve(nodeCount);
    while (!stack.isEmpty()) {
        int u = stack.pop();
        order.append(u);
        for (int v : adj[u]) {
            if (--inDegree[v] == 0) {
                stack.push(v);
            }
        }
    }
    return order;
}

/** @brief 重置统计 */
void StronglyConnected::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Tarjan算法实现 @param adjList 邻接表 @return SCC结果 */
StronglyConnected::SccResult StronglyConnected::tarjanSCC(
    const QVector<QVector<int>>& adjList)
{
    int n = adjList.size();
    QVector<int> indices(n, -1);
    QVector<int> lowlink(n, -1);
    QVector<bool> onStack(n, false);
    QVector<int> stack;
    QVector<QVector<int>> components;
    int index = 0;

    /* 用lambda实现DFS避免递归过深 */
    /* Tarjan核心DFS */
    for (int v = 0; v < n; ++v) {
        if (indices[v] == -1) {
            tarjanDFS(v, adjList, index, stack, onStack,
                      indices, lowlink, components);
        }
    }

    SccResult result;
    result.components = components;
    return result;
}

/** @brief Tarjan DFS @param u 当前顶点 @param adjList 邻接表 @param index 编号 @param stack 栈 @param onStack 在栈标记 @param indices 编号数组 @param lowlink 低链接值 @param result 结果 */
void StronglyConnected::tarjanDFS(int u, const QVector<QVector<int>>& adjList,
                                   int& index, QVector<int>& stack,
                                   QVector<bool>& onStack,
                                   QVector<int>& indices,
                                   QVector<int>& lowlink,
                                   QVector<QVector<int>>& result)
{
    indices[u] = index;
    lowlink[u] = index;
    ++index;
    stack.append(u);
    onStack[u] = true;

    for (int v : adjList[u]) {
        if (v < 0 || v >= adjList.size()) continue;

        if (indices[v] == -1) {
            tarjanDFS(v, adjList, index, stack, onStack,
                      indices, lowlink, result);
            lowlink[u] = qMin(lowlink[u], lowlink[v]);
        } else if (onStack[v]) {
            lowlink[u] = qMin(lowlink[u], indices[v]);
        }
    }

    /* 如果u是SCC根节点，弹出栈中该SCC所有节点 */
    if (lowlink[u] == indices[u]) {
        QVector<int> component;
        int w;
        do {
            w = stack.takeLast();
            onStack[w] = false;
            component.append(w);
        } while (w != u);
        result.append(component);
    }
}

/** @brief Kosaraju算法实现 @param adjList 邻接表 @return SCC结果 */
StronglyConnected::SccResult StronglyConnected::kosarajuSCC(
    const QVector<QVector<int>>& adjList)
{
    int n = adjList.size();

    /* 构建逆图 */
    QVector<QVector<int>> radj(n);
    for (int u = 0; u < n; ++u) {
        for (int v : adjList[u]) {
            if (v >= 0 && v < n) {
                radj[v].append(u);
            }
        }
    }

    /* 第一次DFS: 计算逆后序 */
    QVector<bool> visited(n, false);
    QVector<int> order;
    for (int v = 0; v < n; ++v) {
        if (!visited[v]) {
            kosarajuDFS1(v, adjList, visited, order);
        }
    }

    /* 第二次DFS: 按逆后序在逆图上遍历 */
    QVector<QVector<int>> components;
    visited.fill(false);
    for (int i = order.size() - 1; i >= 0; --i) {
        int v = order[i];
        if (!visited[v]) {
            QVector<int> component;
            kosarajuDFS2(v, radj, visited, component);
            components.append(component);
        }
    }

    SccResult result;
    result.components = components;
    return result;
}

/** @brief Kosaraju逆图DFS @param u 当前顶点 @param adj 邻接表 @param visited 访问标记 @param order 逆后序 */
void StronglyConnected::kosarajuDFS1(int u, const QVector<QVector<int>>& adj,
                                      QVector<bool>& visited, QVector<int>& order)
{
    visited[u] = true;
    for (int v : adj[u]) {
        if (v >= 0 && v < adj.size() && !visited[v]) {
            kosarajuDFS1(v, adj, visited, order);
        }
    }
    order.append(u);
}

/** @brief Kosaraju正向DFS @param u 当前顶点 @param radj 逆邻接表 @param visited 访问标记 @param component 当前SCC */
void StronglyConnected::kosarajuDFS2(int u, const QVector<QVector<int>>& radj,
                                      QVector<bool>& visited,
                                      QVector<int>& component)
{
    visited[u] = true;
    component.append(u);
    for (int v : radj[u]) {
        if (v >= 0 && v < radj.size() && !visited[v]) {
            kosarajuDFS2(v, radj, visited, component);
        }
    }
}
