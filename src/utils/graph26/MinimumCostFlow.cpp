/**
 * @file MinimumCostFlow.cpp
 * @brief 最小费用流算法实现 — SPFA + 逐次最短增广路径
 */

#include "utils/graph26/MinimumCostFlow.h"

#include <QElapsedTimer>
#include <QtMath>

#include <climits>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
MinimumCostFlow::MinimumCostFlow(QObject* parent)
    : QObject(parent)
    , m_nodeCount(0)
{
}

/** @brief 初始化网络 @param nodeCount 节点数量 */
void MinimumCostFlow::init(int nodeCount)
{
    m_nodeCount = nodeCount;
    m_graph.clear();
    m_graph.resize(nodeCount);
}

/** @brief 添加有向边 @param from 起点 @param to 终点 @param capacity 容量 @param cost 费用 */
void MinimumCostFlow::addEdge(int from, int to, int capacity, int cost)
{
    if (from < 0 || from >= m_nodeCount || to < 0 || to >= m_nodeCount) {
        return;
    }

    /* 正向边 */
    Edge forward{to, capacity, cost, static_cast<int>(m_graph[to].size())};
    /* 反向边 (初始容量 0，费用取反) */
    Edge backward{from, 0, -cost, static_cast<int>(m_graph[from].size())};

    m_graph[from].append(forward);
    m_graph[to].append(backward);
}

/** @brief 求解最小费用最大流 @param source 源点 @param sink 汇点 @param maxFlow 最大流量上限 */
MinimumCostFlow::FlowResult MinimumCostFlow::solve(int source, int sink, int maxFlow)
{
    QElapsedTimer timer;
    timer.start();

    FlowResult result;
    if (m_nodeCount <= 0 || source < 0 || sink < 0
        || source >= m_nodeCount || sink >= m_nodeCount) {
        return result;
    }

    int flow = 0, cost = 0;
    int totalEdges = 0;
    for (const auto& adj : m_graph) {
        totalEdges += adj.size();
    }

    int augmentCount = 0;

    while (flow < maxFlow) {
        QVector<int> dist(m_nodeCount, INT_MAX);
        QVector<int> parentNode(m_nodeCount, -1);
        QVector<int> parentEdge(m_nodeCount, -1);

        if (!spfa(source, sink, dist, parentNode, parentEdge)) {
            break;
        }

        /* 沿增广路径计算可增加流量 */
        int addFlow = maxFlow - flow;
        int cur = sink;
        while (cur != source) {
            int p = parentNode[cur];
            int e = parentEdge[cur];
            addFlow = qMin(addFlow, m_graph[p][e].capacity);
            cur = p;
        }

        /* 更新残量网络 */
        cur = sink;
        while (cur != source) {
            int p = parentNode[cur];
            int e = parentEdge[cur];
            m_graph[p][e].capacity -= addFlow;
            m_graph[cur][m_graph[p][e].rev].capacity += addFlow;
            cur = p;
        }

        flow += addFlow;
        cost += addFlow * dist[sink];
        ++augmentCount;
    }

    result.totalFlow = flow;
    result.totalCost = cost;
    result.feasible = (flow > 0);

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSolves;
    m_stats.totalAugments += augmentCount;
    m_stats.totalNodes += m_nodeCount;
    m_stats.totalEdges += totalEdges / 2;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solved(flow, cost);
    return result;
}

/** @brief 获取指定边的流量 @param from 起点 @param edgeIndex 边索引 */
int MinimumCostFlow::getFlow(int from, int edgeIndex) const
{
    if (from < 0 || from >= m_nodeCount) return 0;
    if (edgeIndex < 0 || edgeIndex >= m_graph[from].size()) return 0;

    /* 流量 = 初始容量 - 当前残量，但需从反向边读取 */
    int revIdx = m_graph[from][edgeIndex].rev;
    int to = m_graph[from][edgeIndex].to;
    return m_graph[to][revIdx].capacity;
}

/** @brief 重置统计信息 */
void MinimumCostFlow::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief SPFA 最短路算法 (支持负边权)
 * @param source 源点
 * @param sink 汇点
 * @param dist 距离数组
 * @param parentNode 父节点数组
 * @param parentEdge 父边数组
 * @return 是否存在增广路径
 */
bool MinimumCostFlow::spfa(int source, int sink,
                           QVector<int>& dist,
                           QVector<int>& parentNode,
                           QVector<int>& parentEdge)
{
    dist[source] = 0;
    QVector<bool> inQueue(m_nodeCount, false);
    std::queue<int> q;
    q.push(source);
    inQueue[source] = true;
    QVector<int> count(m_nodeCount, 0);
    count[source] = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        inQueue[u] = false;

        for (int i = 0; i < m_graph[u].size(); ++i) {
            const Edge& e = m_graph[u][i];
            if (e.capacity > 0 && dist[u] + e.cost < dist[e.to]) {
                dist[e.to] = dist[u] + e.cost;
                parentNode[e.to] = u;
                parentEdge[e.to] = i;

                if (!inQueue[e.to]) {
                    q.push(e.to);
                    inQueue[e.to] = true;
                    ++count[e.to];
                    /* 负环检测 */
                    if (count[e.to] > m_nodeCount) {
                        return false;
                    }
                }
            }
        }
    }

    return dist[sink] != INT_MAX;
}
