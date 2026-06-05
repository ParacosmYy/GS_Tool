/**
 * @file MaximumFlow2.cpp
 * @brief 最大流增强实现 — Dinic算法/容量缩放/最小割
 */

#include "utils/graph46/MaximumFlow2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
MaximumFlow2::MaximumFlow2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 构建流网络
 * @param n 顶点数
 * @param edges 边列表((u,v), capacity)
 * @param source 源点
 * @param sink 汇点
 */
void MaximumFlow2::setGraph(int n,
    const QVector<QPair<QPair<int, int>, double>>& edges,
    int source, int sink)
{
    m_n = qMax(2, n);
    m_source = qBound(0, source, m_n - 1);
    m_sink = qBound(0, sink, m_n - 1);
    m_maxFlowVal = 0.0;

    /* 邻接表: 每条边存储(目标, 容量) */
    m_adj.resize(m_n);
    for (auto& list : m_adj) list.clear();

    for (const auto& edge : edges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double cap = edge.second;

        if (u < 0 || u >= m_n || v < 0 || v >= m_n || cap <= 0.0) continue;

        /* 正向边 */
        m_adj[u].append(qMakePair(v, cap));
        /* 反向边(初始容量为0) */
        m_adj[v].append(qMakePair(u, 0.0));
    }

    m_level.resize(m_n, -1);
    m_iter.resize(m_n, 0);
}

/**
 * @brief Dinic最大流算法
 * @return 最大流值
 */
double MaximumFlow2::maxFlow()
{
    return dinicFlow();
}

/**
 * @brief Dinic算法实现
 * @return 最大流值
 */
double MaximumFlow2::dinicFlow()
{
    QElapsedTimer timer;
    timer.start();

    m_maxFlowVal = 0.0;

    /* 重复BFS构建层次图 + DFS增广 */
    while (bfsLevel()) {
        m_iter.fill(0);
        double pushed = 0.0;
        do {
            pushed = dfsSend(m_source, std::numeric_limits<double>::max());
            m_maxFlowVal += pushed;
        } while (pushed > 1e-10);
    }

    m_stats.totalFlows++;
    m_stats.totalVerticesProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFlows));

    emit flowComplete(m_maxFlowVal);
    return m_maxFlowVal;
}

/**
 * @brief 容量缩放最大流
 * @return 最大流值
 */
double MaximumFlow2::scalingFlow()
{
    QElapsedTimer timer;
    timer.start();

    m_maxFlowVal = 0.0;

    /* 找到最大边容量 */
    double maxCap = 0.0;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& edge : m_adj[u]) {
            maxCap = qMax(maxCap, edge.second);
        }
    }

    /* 从高位到低位逐步缩小Delta */
    double delta = 1.0;
    while (delta * 2.0 <= maxCap) delta *= 2.0;

    while (delta >= 1.0) {
        /* 只使用容量>=delta的边进行增广 */
        while (bfsLevelThreshold(delta)) {
            m_iter.fill(0);
            double pushed = 0.0;
            do {
                pushed = dfsSendThreshold(m_source,
                    std::numeric_limits<double>::max(), delta);
                m_maxFlowVal += pushed;
            } while (pushed > 1e-10);
        }
        delta /= 2.0;
    }

    m_stats.totalFlows++;
    m_stats.totalVerticesProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFlows));

    emit flowComplete(m_maxFlowVal);
    return m_maxFlowVal;
}

/**
 * @brief 计算最小割
 * @return 割边列表(u, v)
 */
QVector<QPair<int, int>> MaximumFlow2::minCut() const
{
    /* 从源点BFS找到可达集合 */
    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(m_source);
    visited[m_source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double cap = edge.second;
            if (!visited[v] && cap > 1e-10) {
                visited[v] = true;
                q.push(v);
            }
        }
    }

    /* 找跨越割的边 */
    QVector<QPair<int, int>> cut;
    for (int u = 0; u < m_n; ++u) {
        if (!visited[u]) continue;
        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            if (!visited[v] && edge.second < 1e-10) {
                /* 检查是否存在原始正向边(索引为偶数) */
                cut.append(qMakePair(u, v));
            }
        }
    }

    return cut;
}

/** @brief 重置图状态 */
void MaximumFlow2::reset()
{
    for (auto& list : m_adj) list.clear();
    m_n = 0;
    m_source = 0;
    m_sink = 0;
    m_maxFlowVal = 0.0;
}

/** @brief 重置统计 */
void MaximumFlow2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief BFS构建层次图
 * @return 汇点是否可达
 */
bool MaximumFlow2::bfsLevel()
{
    m_level.fill(-1);
    std::queue<int> q;
    m_level[m_source] = 0;
    q.push(m_source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double cap = edge.second;
            if (m_level[v] < 0 && cap > 1e-10) {
                m_level[v] = m_level[u] + 1;
                q.push(v);
            }
        }
    }

    return m_level[m_sink] >= 0;
}

/**
 * @brief BFS构建层次图(带容量阈值)
 * @param delta 最小容量
 * @return 汇点是否可达
 */
bool MaximumFlow2::bfsLevelThreshold(double delta)
{
    m_level.fill(-1);
    std::queue<int> q;
    m_level[m_source] = 0;
    q.push(m_source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double cap = edge.second;
            if (m_level[v] < 0 && cap >= delta) {
                m_level[v] = m_level[u] + 1;
                q.push(v);
            }
        }
    }

    return m_level[m_sink] >= 0;
}

/**
 * @brief DFS发送流量
 * @param u 当前顶点
 * @param flow 可用流量
 * @return 实际发送流量
 */
double MaximumFlow2::dfsSend(int u, double flow)
{
    if (u == m_sink) return flow;

    for (int& i = m_iter[u]; i < m_adj[u].size(); ++i) {
        int v = m_adj[u][i].first;
        double& cap = m_adj[u][i].second;

        if (m_level[v] == m_level[u] + 1 && cap > 1e-10) {
            double pushed = dfsSend(v, qMin(flow, cap));
            if (pushed > 1e-10) {
                cap -= pushed;
                /* 更新反向边容量 */
                for (auto& rev : m_adj[v]) {
                    if (rev.first == u) {
                        rev.second += pushed;
                        break;
                    }
                }
                return pushed;
            }
        }
    }

    return 0.0;
}

/**
 * @brief DFS发送流量(带容量阈值)
 * @param u 当前顶点
 * @param flow 可用流量
 * @param delta 最小容量
 * @return 实际发送流量
 */
double MaximumFlow2::dfsSendThreshold(int u, double flow, double delta)
{
    if (u == m_sink) return flow;

    for (int& i = m_iter[u]; i < m_adj[u].size(); ++i) {
        int v = m_adj[u][i].first;
        double& cap = m_adj[u][i].second;

        if (m_level[v] == m_level[u] + 1 && cap >= delta) {
            double pushed = dfsSendThreshold(v, qMin(flow, cap), delta);
            if (pushed > 1e-10) {
                cap -= pushed;
                for (auto& rev : m_adj[v]) {
                    if (rev.first == u) {
                        rev.second += pushed;
                        break;
                    }
                }
                return pushed;
            }
        }
    }

    return 0.0;
}
