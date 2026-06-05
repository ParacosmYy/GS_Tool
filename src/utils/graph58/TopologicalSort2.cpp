/**
 * @file TopologicalSort2.cpp
 * @brief 拓扑排序2实现 — Kahn+BFS+最长路径
 *
 * 拓扑排序实现:
 * - Kahn算法: 基于入度的BFS拓扑排序
 * - DFS排序: 基于深度优先搜索的拓扑排序
 * - 环检测: 判断有向图是否有环
 * - 全拓扑序列: 枚举所有合法的拓扑排序
 * - 最长路径: DAG上的最长路径(DP)
 * - 关键路径: 项目管理中的关键路径法
 *
 * 统计信息跟踪: 排序次数、顶点数、平均耗时。
 */

#include "utils/graph58/TopologicalSort2.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>
#include <stack>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
TopologicalSort2::TopologicalSort2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置有向图
 * @param n 顶点数
 * @param edges 有向边列表(from, to)
 */
void TopologicalSort2::setGraph(int n, const QVector<QPair<int,int>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    m_inDegree.resize(n, 0);

    for (auto& adj : m_adj) adj.clear();
    std::fill(m_inDegree.begin(), m_inDegree.end(), 0);

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_adj[e.first].append(e.second);
            m_inDegree[e.second]++;
        }
    }
}

/**
 * @brief 执行拓扑排序(默认使用Kahn算法)
 * @return 排序结果(有环时返回空)
 */
QVector<int> TopologicalSort2::sort()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result = kahnSort();

    // 更新统计
    m_stats.totalSorts++;
    m_stats.totalVertices += m_n;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(m_n, result.size() < m_n);
    return result;
}

/**
 * @brief Kahn算法拓扑排序
 *
 * 算法流程:
 * 1. 将所有入度为0的顶点入队
 * 2. 取出队首顶点，加入结果
 * 3. 将其所有邻居的入度减1
 * 4. 若邻居入度变为0，入队
 * 5. 重复直到队列为空
 *
 * 若结果长度 < 顶点数，说明存在环。
 *
 * @return 排序结果(有环返回空)
 */
QVector<int> TopologicalSort2::kahnSort() const
{
    QVector<int> inDeg = m_inDegree;
    QVector<int> result;
    result.reserve(m_n);

    std::queue<int> q;
    for (int i = 0; i < m_n; ++i) {
        if (inDeg[i] == 0) {
            q.push(i);
        }
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        result.append(u);

        for (int v : m_adj[u]) {
            inDeg[v]--;
            if (inDeg[v] == 0) {
                q.push(v);
            }
        }
    }

    if (result.size() < m_n) {
        return {};  ///< 存在环
    }
    return result;
}

/**
 * @brief DFS拓扑排序
 *
 * 按DFS完成时间的逆序排列:
 * 1. 访问未访问的顶点
 * 2. 递归访问其所有邻居
 * 3. 回溯时将顶点加入结果(逆序)
 *
 * @return 排序结果(有环返回空)
 */
QVector<int> TopologicalSort2::dfsSort() const
{
    QVector<bool> visited(m_n, false);
    QVector<bool> onStack(m_n, false);  ///< 检测环用
    QVector<int> result;
    bool hasCycle = false;

    for (int i = 0; i < m_n; ++i) {
        if (!visited[i]) {
            dfsVisit(i, visited, onStack, result, hasCycle);
        }
    }

    if (hasCycle) return {};

    // 逆序即为拓扑序列
    std::reverse(result.begin(), result.end());
    return result;
}

/**
 * @brief DFS辅助函数
 * @param u 当前顶点
 * @param visited 已访问标记
 * @param onStack 当前递归栈上的标记(环检测)
 * @param result DFS完成序列
 * @param cycle 是否检测到环
 */
void TopologicalSort2::dfsVisit(int u, QVector<bool>& visited,
                                  QVector<bool>& onStack,
                                  QVector<int>& result, bool& cycle) const
{
    visited[u] = true;
    onStack[u] = true;

    for (int v : m_adj[u]) {
        if (onStack[v]) {
            cycle = true;  ///< 发现环
            return;
        }
        if (!visited[v]) {
            dfsVisit(v, visited, onStack, result, cycle);
            if (cycle) return;
        }
    }

    onStack[u] = false;
    result.append(u);
}

/**
 * @brief 检测图中是否有环
 * @return true如果有环
 */
bool TopologicalSort2::hasCycle() const
{
    QVector<int> result = kahnSort();
    return result.size() < m_n;
}

/**
 * @brief 枚举所有拓扑排序
 *
 * 使用回溯法枚举所有合法的拓扑序列:
 * 1. 维护当前入度为0的候选集合
 * 2. 依次尝试每个候选
 * 3. 递归处理剩余顶点
 * 4. 限制最大结果数防止指数爆炸
 *
 * @return 所有拓扑排序(最多1000个)
 */
QVector<QVector<int>> TopologicalSort2::allTopologicalSorts() const
{
    if (hasCycle()) return {};

    QVector<QVector<int>> allResults;
    QVector<int> result;
    QVector<bool> visited(m_n, false);
    QVector<int> inDeg = m_inDegree;

    allSortsDFS(result, visited, inDeg, allResults, 1000);
    return allResults;
}

/**
 * @brief 全拓扑排序的DFS回溯
 * @param result 当前部分结果
 * @param visited 已访问标记
 * @param inDeg 当前入度
 * @param allResults 收集所有结果
 * @param limit 最大结果数
 */
void TopologicalSort2::allSortsDFS(QVector<int>& result,
                                     QVector<bool>& visited,
                                     QVector<int>& inDeg,
                                     QVector<QVector<int>>& allResults,
                                     int limit) const
{
    if (allResults.size() >= limit) return;

    if (result.size() == m_n) {
        allResults.append(result);
        return;
    }

    // 找所有入度为0且未访问的顶点
    for (int u = 0; u < m_n; ++u) {
        if (!visited[u] && inDeg[u] == 0) {
            // 选择u
            visited[u] = true;
            result.append(u);

            // 更新邻居入度
            for (int v : m_adj[u]) {
                inDeg[v]--;
            }

            allSortsDFS(result, visited, inDeg, allResults, limit);

            // 撤销选择
            for (int v : m_adj[u]) {
                inDeg[v]++;
            }
            result.removeLast();
            visited[u] = false;

            if (allResults.size() >= limit) return;
        }
    }
}

/**
 * @brief 计算DAG上的最长路径
 *
 * 使用动态规划:
 * dist[v] = max(dist[u] + 1) for all edges (u,v)
 *
 * @return 最长路径的长度
 */
int TopologicalSort2::longestPath() const
{
    if (hasCycle()) return -1;

    QVector<int> topo = kahnSort();
    if (topo.isEmpty()) return -1;

    QVector<int> dist(m_n, 0);

    for (int u : topo) {
        for (int v : m_adj[u]) {
            dist[v] = qMax(dist[v], dist[u] + 1);
        }
    }

    int maxDist = 0;
    for (int d : dist) {
        maxDist = qMax(maxDist, d);
    }
    return maxDist;
}

/**
 * @brief 计算关键路径
 *
 * 关键路径是DAG中从源到汇的最长路径，
 * 决定了项目的最短完成时间。
 *
 * 使用正/逆向传播计算:
 * - ES (Early Start): 最早开始时间
 * - EF (Early Finish): 最早完成时间
 * - LS (Late Start): 最晚开始时间
 * - LF (Late Finish): 最晚完成时间
 *
 * 关键活动: ES == LS 的活动(没有松弛时间)
 *
 * @return 关键路径上的顶点序列
 */
QVector<int> TopologicalSort2::criticalPath() const
{
    if (hasCycle()) return {};

    QVector<int> topo = kahnSort();
    if (topo.isEmpty()) return {};

    // 正向传播: 计算ES
    QVector<int> ES(m_n, 0);
    for (int u : topo) {
        for (int v : m_adj[u]) {
            ES[v] = qMax(ES[v], ES[u] + 1);
        }
    }

    // 项目总工期
    int projectLen = 0;
    for (int d : ES) projectLen = qMax(projectLen, d);

    // 逆向传播: 计算LS
    QVector<int> LS(m_n, projectLen);
    for (int i = topo.size() - 1; i >= 0; --i) {
        int u = topo[i];
        for (int v : m_adj[u]) {
            LS[u] = qMin(LS[u], LS[v] - 1);
        }
    }

    // 关键路径: ES == LS 的顶点
    QVector<int> critical;
    for (int u : topo) {
        if (ES[u] == LS[u]) {
            critical.append(u);
        }
    }

    return critical;
}

/**
 * @brief 重置所有统计信息
 */
void TopologicalSort2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
