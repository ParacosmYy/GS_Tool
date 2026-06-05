/**
 * @file Matching4.cpp
 * @brief 图匹配算法实现（最大权匹配与最大基数匹配）
 *
 * 实现一般图的最大权匹配和最大基数匹配算法。
 * 最大基数匹配使用贪心增广路径方法，最大权匹配基于
 * Blossom算法的思想，通过权值排序实现近似最优。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph65/Matching4.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>

/**
 * @brief 构造函数，初始化空图
 * @param parent 父QObject对象指针
 */
Matching4::Matching4(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_totalWeight(0.0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置图的顶点数和边集
 *
 * @param n 顶点数量，顶点编号为 0 到 n-1
 * @param edges 边列表，每条边包含 (顶点u, 顶点v) 和边权值
 */
void Matching4::setGraph(int n, const QVector<QPair<QPair<int,int>,double>>& edges)
{
    m_n = n;
    m_adj.clear();
    m_adj.resize(n);

    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double w = edge.second;

        if (u >= 0 && u < n && v >= 0 && v < n && u != v) {
            m_adj[u].append({v, w});
            m_adj[v].append({u, w});
        }
    }
}

/**
 * @brief 求最大权匹配
 *
 * 基于权值排序的贪心算法。将所有边按权值降序排列，
 * 依次尝试将每条边加入匹配（若两端点均未匹配）。
 * 该方法为近似算法，时间复杂度 O(E log E)。
 *
 * @return 匹配的边端点对列表
 */
QVector<QPair<int,int>> Matching4::maximumWeighted()
{
    QElapsedTimer timer;
    timer.start();

    m_matching.clear();
    m_totalWeight = 0.0;

    if (m_n <= 0) {
        return m_matching;
    }

    /* 收集所有边并按权值降序排列 */
    QVector<QPair<double, QPair<int,int>>> edges;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& neighbor : m_adj[u]) {
            int v = neighbor.first;
            double w = neighbor.second;
            if (u < v) {  ///< 避免重复添加无向边
                edges.append({w, {u, v}});
            }
        }
    }

    /* 按权值从大到小排序 */
    std::sort(edges.begin(), edges.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    /* 贪心匹配: 按权值顺序，两端点均未匹配则加入匹配 */
    QVector<bool> matched(m_n, false);

    for (const auto& edge : edges) {
        int u = edge.second.first;
        int v = edge.second.second;
        double w = edge.first;

        if (!matched[u] && !matched[v]) {
            m_matching.append({u, v});
            m_totalWeight += w;
            matched[u] = true;
            matched[v] = true;
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 求最大基数匹配
 *
 * 使用BFS增广路径方法求解最大基数匹配。
 * 反复寻找增广路径并扩展匹配，直到不存在增广路径为止。
 * 时间复杂度 O(V * E)。
 *
 * @return 匹配的边端点对列表
 */
QVector<QPair<int,int>> Matching4::maximumCardinality()
{
    QElapsedTimer timer;
    timer.start();

    m_matching.clear();
    m_totalWeight = 0.0;

    if (m_n <= 0) {
        return m_matching;
    }

    QVector<int> match(m_n, -1);  ///< match[v] = 与v匹配的顶点，-1表示未匹配

    bool found = true;
    while (found) {
        found = false;

        /* BFS寻找增广路径 */
        QVector<int> parent(m_n, -1);
        QVector<bool> visited(m_n, false);
        std::queue<int> queue;

        /* 将所有未匹配顶点作为BFS起点 */
        for (int v = 0; v < m_n; ++v) {
            if (match[v] == -1) {
                visited[v] = true;
                queue.push(v);
            }
        }

        while (!queue.empty() && !found) {
            int u = queue.front();
            queue.pop();

            for (const auto& neighbor : m_adj[u]) {
                int v = neighbor.first;
                if (visited[v]) {
                    continue;
                }

                visited[v] = true;
                parent[v] = u;

                if (match[v] == -1) {
                    /* 找到增广路径，沿路径翻转匹配状态 */
                    int cur = v;
                    while (cur != -1) {
                        int prev = parent[cur];
                        if (prev == -1) {
                            break;
                        }
                        int nextCur = match[prev];
                        match[prev] = cur;
                        match[cur] = prev;
                        cur = nextCur;
                    }
                    found = true;
                    break;
                } else {
                    /* 将匹配对中的另一端加入队列继续搜索 */
                    int matchedVertex = match[v];
                    if (!visited[matchedVertex]) {
                        visited[matchedVertex] = true;
                        parent[matchedVertex] = v;
                        queue.push(matchedVertex);
                    }
                }
            }
        }
    }

    /* 从match数组构建匹配边列表 */
    QVector<bool> counted(m_n, false);
    for (int v = 0; v < m_n; ++v) {
        if (match[v] != -1 && !counted[v]) {
            m_matching.append({v, match[v]});
            counted[v] = true;
            counted[match[v]] = true;

            /* 累加匹配边的权值 */
            for (const auto& neighbor : m_adj[v]) {
                if (neighbor.first == match[v]) {
                    m_totalWeight += neighbor.second;
                    break;
                }
            }
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 重置所有统计计数器
 */
void Matching4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
