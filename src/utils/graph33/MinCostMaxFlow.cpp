/**
 * @file MinCostMaxFlow.cpp
 * @brief 最小费用最大流实现 — SPFA增广/费用缩放/对偶变量
 */

#include "utils/graph33/MinCostMaxFlow.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
MinCostMaxFlow::MinCostMaxFlow(QObject* parent)
    : QObject(parent)
    , m_vertexCount(0)
{
}

/** @brief 初始化图 @param n 顶点数 */
void MinCostMaxFlow::init(int n)
{
    m_vertexCount = n;
    m_graph.assign(n, QVector<Edge>());
    m_potential.assign(n, 0.0);
}

/** @brief 添加边 @param from 源 @param to 目标 @param capacity 容量 @param cost 费用 */
void MinCostMaxFlow::addEdge(int from, int to, double capacity, double cost)
{
    if (from < 0 || from >= m_vertexCount || to < 0 || to >= m_vertexCount) {
        return;
    }

    /* 正向边 */
    Edge forward{to, m_graph[to].size(), capacity, cost};
    /* 反向边 */
    Edge backward{from, m_graph[from].size(), 0.0, -cost};

    m_graph[from].append(forward);
    m_graph[to].append(backward);
}

/** @brief 依次最短路增广求解 @param source 源点 @param sink 汇点 @param maxFlow 最大流量限制 */
MinCostMaxFlow::FlowResult MinCostMaxFlow::successiveShortestPath(
    int source, int sink, double maxFlow)
{
    QElapsedTimer timer;
    timer.start();

    FlowResult result;
    double remainingFlow = maxFlow;

    /* 初始化对偶变量(势函数) */
    m_potential.assign(m_vertexCount, 0.0);

    while (remainingFlow > 1e-10) {
        /* SPFA求最短路 */
        auto pair = spfa(source, sink, m_vertexCount);
        QVector<double>& dist = pair.first;
        QVector<int>& parent = pair.second;

        /* 无法到达汇点 */
        if (dist[sink] >= 1e17) break;

        /* 沿最路增广 */
        double augment = augmentAlongPath(source, sink, parent);
        augment = qMin(augment, remainingFlow);

        if (augment < 1e-10) break;

        /* 更新势函数 */
        for (int i = 0; i < m_vertexCount; ++i) {
            if (dist[i] < 1e17) {
                m_potential[i] += dist[i];
            }
        }

        result.totalFlow += augment;
        result.totalCost += augment * dist[sink];
        result.augmentations++;
        remainingFlow -= augment;

        emit augmentDone(augment, augment * dist[sink]);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalSolves;
    m_stats.totalAugmentations += result.augmentations;
    int edgeCount = 0;
    for (const auto& adj : m_graph) edgeCount += adj.size();
    m_stats.totalEdgesProcessed += edgeCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    double efficiency = (result.totalCost > 0)
        ? result.totalFlow / result.totalCost : 0.0;
    if (efficiency > m_stats.bestCostEfficiency) {
        m_stats.bestCostEfficiency = efficiency;
    }

    emit solveComplete(result.totalFlow, result.totalCost);
    return result;
}

/** @brief SPFA最短路 @param source 源 @param sink 汇 @param n 顶点数 */
QPair<QVector<double>, QVector<int>> MinCostMaxFlow::spfa(
    int source, int sink, int n)
{
    QVector<double> dist(n, 1e18);
    QVector<int> parent(n, -1);
    QVector<int> parentEdge(n, -1);
    QVector<bool> inQueue(n, false);
    QVector<int> count(n, 0);

    dist[source] = 0.0;
    std::queue<int> q;
    q.push(source);
    inQueue[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        inQueue[u] = false;

        for (int i = 0; i < m_graph[u].size(); ++i) {
            const Edge& e = m_graph[u][i];
            if (e.capacity < 1e-10) continue;

            /* 使用reduced cost加速 */
            double rc = e.cost + m_potential[u] - m_potential[e.to];
            double newDist = dist[u] + rc;

            if (newDist < dist[e.to] - 1e-10) {
                dist[e.to] = newDist;
                parent[e.to] = u;
                parentEdge[e.to] = i;

                if (!inQueue[e.to]) {
                    ++count[e.to];
                    if (count[e.to] > n) {
                        /* 检测到负环，返回当前结果 */
                        return {dist, parent};
                    }
                    q.push(e.to);
                    inQueue[e.to] = true;
                }
            }
        }
    }

    return {dist, parent};
}

/** @brief 费用缩放求解 @param source 源 @param sink 汇 @param epsilon 精度 */
MinCostMaxFlow::FlowResult MinCostMaxFlow::costScaling(
    int source, int sink, double epsilon)
{
    QElapsedTimer timer;
    timer.start();

    FlowResult result;

    /* 费用缩放: 从粗粒度到细粒度逐步求解 */
    double delta = 1.0;
    /* 找到最大绝对费用作为初始缩放因子 */
    for (const auto& adj : m_graph) {
        for (const Edge& e : adj) {
            if (e.cost > delta) delta = e.cost;
            if (-e.cost > delta) delta = -e.cost;
        }
    }

    while (delta >= epsilon) {
        /* 缩放后的残量图 */
        for (auto& adj : m_graph) {
            for (Edge& e : adj) {
                /* 四舍五入费用到delta精度 */
                e.cost = qRound(e.cost / delta) * delta;
            }
        }

        /* 在缩放后的图上求解 */
        FlowResult partial = successiveShortestPath(source, sink);
        result.totalFlow += partial.totalFlow;
        result.totalCost += partial.totalCost;
        result.augmentations += partial.augmentations;

        delta *= 0.5;
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveComplete(result.totalFlow, result.totalCost);
    return result;
}

/** @brief 获取对偶变量(势函数) @return 各顶点势值 */
QVector<double> MinCostMaxFlow::dualVariables() const
{
    return m_potential;
}

/** @brief 重置统计 */
void MinCostMaxFlow::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_graph.clear();
    m_potential.clear();
    m_vertexCount = 0;
}

/** @brief 沿路径增广 @param source 源 @param sink 汇 @param parent 前驱数组 */
double MinCostMaxFlow::augmentAlongPath(int source, int sink,
                                        const QVector<int>& parent)
{
    /* 找瓶颈容量 */
    double minCap = 1e18;
    int v = sink;
    while (v != source && parent[v] >= 0) {
        int u = parent[v];
        /* 找到u→v的边 */
        for (const Edge& e : m_graph[u]) {
            if (e.to == v && e.capacity > 0) {
                minCap = qMin(minCap, e.capacity);
                break;
            }
        }
        v = u;
    }
    if (v != source) return 0.0;

    /* 增广 */
    v = sink;
    while (v != source) {
        int u = parent[v];
        for (Edge& e : m_graph[u]) {
            if (e.to == v && e.capacity > 0) {
                e.capacity -= minCap;
                m_graph[v][e.rev].capacity += minCap;
                break;
            }
        }
        v = u;
    }

    return minCap;
}
