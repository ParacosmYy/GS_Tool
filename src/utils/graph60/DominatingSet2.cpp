/**
 * @file DominatingSet2.cpp
 * @brief 支配集2实现 — 贪心近似+加权支配
 *
 * 实现最小支配集问题的贪心近似算法，支持均匀权重和加权版本。
 * 加权版本以 min(weight/degree) 为贪心准则。
 * 还支持连通支配集的构造。
 */

#include "utils/graph60/DominatingSet2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化空图
 * @param parent 父QObject
 */
DominatingSet2::DominatingSet2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的邻接关系
 * @param n 顶点数
 * @param edges 边列表，每条边为(u,v)无向边
 */
void DominatingSet2::setGraph(int n, const QVector<QPair<int,int>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    for (auto& adj : m_adj) adj.clear();

    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (u >= 0 && u < n && v >= 0 && v < n && u != v) {
            /* 避免重复边 */
            bool exists = false;
            for (int nb : m_adj[u]) {
                if (nb == v) { exists = true; break; }
            }
            if (!exists) {
                m_adj[u].append(v);
                m_adj[v].append(u);
            }
        }
    }

    /* 默认均匀权重 */
    m_weights.resize(n);
    m_weights.fill(1.0);
}

/**
 * @brief 设置顶点权重
 * @param weights 顶点权重向量，用于加权支配集
 */
void DominatingSet2::setWeights(const QVector<double>& weights)
{
    m_weights = weights;
    /* 确保维度匹配 */
    if (m_weights.size() < m_n) {
        m_weights.resize(m_n, 1.0);
    }
}

/**
 * @brief 求解最小支配集(无权重)
 * @return 支配集中的顶点索引
 */
QVector<int> DominatingSet2::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_solution = greedyDominating();

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_stats.dominatingSetSize = m_solution.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(m_solution.size(), m_n);
    return m_solution;
}

/**
 * @brief 求解加权最小支配集
 * @return 支配集中的顶点索引
 *
 * 贪心准则: 每步选择 weight(newly_dominated) / cost 最小的顶点。
 * 等效于 min(cost / new_coverage) 的贪心策略。
 */
QVector<int> DominatingSet2::solveWeighted()
{
    QElapsedTimer timer;
    timer.start();

    m_solution = weightedGreedy();

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_stats.dominatingSetSize = m_solution.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(m_solution.size(), m_n);
    return m_solution;
}

/**
 * @brief 检查给定集合是否为支配集
 * @param set 候选支配集
 * @return true表示是支配集
 *
 * 支配集: 每个顶点要么在集合中，要么与集合中某个顶点相邻。
 */
bool DominatingSet2::isDominating(const QVector<int>& set) const
{
    if (m_n == 0) return true;

    QSet<int> domSet;
    for (int v : set) domSet.insert(v);

    QSet<int> dominated;
    for (int v : set) {
        dominated.insert(v);
        for (int nb : m_adj[v]) {
            dominated.insert(nb);
        }
    }

    return dominated.size() == m_n;
}

/**
 * @brief 求解连通支配集
 * @return 连通支配集中的顶点索引
 *
 * 先求最小支配集，再通过最短路径连接不相邻的支配顶点。
 */
QVector<int> DominatingSet2::connectedDominatingSet()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> domSet = greedyDominating();

    if (domSet.size() <= 1) {
        m_solution = domSet;
        return domSet;
    }

    /* 构建支配集的导出子图连通性 */
    QSet<int> inSet;
    for (int v : domSet) inSet.insert(v);

    /* BFS检查支配集是否已连通 */
    QSet<int> visited;
    QList<int> queue;
    queue.append(domSet[0]);
    visited.insert(domSet[0]);

    while (!queue.isEmpty()) {
        int u = queue.takeFirst();
        for (int nb : m_adj[u]) {
            if (inSet.contains(nb) && !visited.contains(nb)) {
                visited.insert(nb);
                queue.append(nb);
            }
        }
    }

    /* 如果不连通，用BFS路径添加连接节点 */
    while (visited.size() < domSet.size()) {
        /* 找未访问的支配顶点 */
        int target = -1;
        for (int v : domSet) {
            if (!visited.contains(v)) {
                target = v;
                break;
            }
        }
        if (target < 0) break;

        /* BFS从已访问集合到目标 */
        QVector<int> prev(m_n, -1);
        QVector<bool> bfsVisited(m_n, false);
        QList<int> bfsQueue;

        for (int v : visited) {
            bfsQueue.append(v);
            bfsVisited[v] = true;
        }

        while (!bfsQueue.isEmpty()) {
            int u = bfsQueue.takeFirst();
            if (u == target) break;

            for (int nb : m_adj[u]) {
                if (!bfsVisited[nb]) {
                    bfsVisited[nb] = true;
                    prev[nb] = u;
                    bfsQueue.append(nb);
                }
            }
        }

        /* 回溯路径并添加中间节点 */
        int cur = target;
        while (cur >= 0 && prev[cur] >= 0) {
            int p = prev[cur];
            if (!inSet.contains(p)) {
                domSet.append(p);
                inSet.insert(p);
            }
            visited.insert(cur);
            cur = p;
        }
        visited.insert(target);
    }

    m_solution = domSet;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_stats.dominatingSetSize = m_solution.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(m_solution.size(), m_n);
    return m_solution;
}

/**
 * @brief 贪心支配集算法（无权重）
 * @return 支配集顶点列表
 *
 * 每步选择能支配最多未支配顶点的顶点。
 */
QVector<int> DominatingSet2::greedyDominating() const
{
    QVector<bool> dominated(m_n, false);
    QVector<int> result;

    while (true) {
        /* 统计未支配顶点数 */
        int undominated = 0;
        for (int i = 0; i < m_n; ++i) {
            if (!dominated[i]) undominated++;
        }
        if (undominated == 0) break;

        /* 选择支配最多未支配顶点的顶点 */
        int bestVertex = -1;
        int bestCoverage = -1;

        for (int v = 0; v < m_n; ++v) {
            int coverage = 0;
            if (!dominated[v]) coverage++; /* 自己 */
            for (int nb : m_adj[v]) {
                if (!dominated[nb]) coverage++;
            }
            if (coverage > bestCoverage) {
                bestCoverage = coverage;
                bestVertex = v;
            }
        }

        if (bestVertex < 0) break;

        /* 加入支配集 */
        result.append(bestVertex);
        dominated[bestVertex] = true;
        for (int nb : m_adj[bestVertex]) {
            dominated[nb] = true;
        }
    }

    return result;
}

/**
 * @brief 加权贪心支配集算法
 * @return 支配集顶点列表
 *
 * 贪心准则: 选择 weight / new_coverage 最小的顶点。
 */
QVector<int> DominatingSet2::weightedGreedy() const
{
    QVector<bool> dominated(m_n, false);
    QVector<int> result;

    while (true) {
        int undominated = 0;
        for (int i = 0; i < m_n; ++i) {
            if (!dominated[i]) undominated++;
        }
        if (undominated == 0) break;

        /* 选择 weight/newCoverage 最小的顶点 */
        int bestVertex = -1;
        double bestRatio = 1e18;

        for (int v = 0; v < m_n; ++v) {
            int coverage = 0;
            if (!dominated[v]) coverage++;
            for (int nb : m_adj[v]) {
                if (!dominated[nb]) coverage++;
            }
            if (coverage == 0) continue;

            double w = (v < m_weights.size()) ? m_weights[v] : 1.0;
            double ratio = w / coverage;
            if (ratio < bestRatio) {
                bestRatio = ratio;
                bestVertex = v;
            }
        }

        if (bestVertex < 0) break;

        result.append(bestVertex);
        dominated[bestVertex] = true;
        for (int nb : m_adj[bestVertex]) {
            dominated[nb] = true;
        }
    }

    return result;
}

/**
 * @brief 重置所有统计信息
 */
void DominatingSet2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
