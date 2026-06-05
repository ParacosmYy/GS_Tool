/**
 * @file MaxFlowPushRelabel.cpp
 * @brief Push-Relabel最大流实现 — 最高标号/间隔/全局重标号优化
 */

#include "utils/graph32/MaxFlowPushRelabel.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MaxFlowPushRelabel::MaxFlowPushRelabel(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_source(0)
    , m_sink(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置顶点数 @param n 顶点数 */
void MaxFlowPushRelabel::setVertexCount(int n)
{
    m_n = qMax(2, n);
    m_adj.resize(m_n);
    m_excess.resize(m_n, 0.0);
    m_height.resize(m_n, 0);
    m_current.resize(m_n, 0);
    m_inQueue.resize(m_n, false);
}

/** @brief 添加边 @param from 起点 @param to 终点 @param capacity 容量 */
void MaxFlowPushRelabel::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n || capacity < 0) return;

    /* 正向边 */
    Edge forward;
    forward.from = from;
    forward.to = to;
    forward.capacity = capacity;
    forward.flow = 0.0;
    forward.rev = m_adj[to].size();

    /* 反向边 */
    Edge backward;
    backward.from = to;
    backward.to = from;
    backward.capacity = 0.0;
    backward.flow = 0.0;
    backward.rev = m_adj[from].size();

    m_adj[from].append(forward);
    m_adj[to].append(backward);
}

/** @brief 清空所有边 */
void MaxFlowPushRelabel::clearEdges()
{
    for (auto& list : m_adj) list.clear();
    m_excess.fill(0.0);
    m_height.fill(0);
    m_current.fill(0);
    m_inQueue.fill(false);
}

/** @brief 初始化预流 @param source 源点 */
void MaxFlowPushRelabel::initPreflow(int source)
{
    m_excess.fill(0.0);
    m_height.fill(0);
    m_current.fill(0);
    m_inQueue.fill(false);

    m_height[source] = m_n;

    /* 从源点推流 */
    for (int i = 0; i < m_adj[source].size(); ++i) {
        auto& edge = m_adj[source][i];
        if (edge.capacity > 0) {
            double pushFlow = edge.capacity;
            edge.flow = pushFlow;
            m_adj[edge.to][edge.rev].flow = -pushFlow;
            m_excess[edge.to] += pushFlow;
            m_excess[source] -= pushFlow;
            m_stats.totalPushes++;
        }
    }
}

/** @brief Push操作 @param v 顶点 @param edgeIdx 边索引 */
void MaxFlowPushRelabel::push(int v, int edgeIdx)
{
    auto& edge = m_adj[v][edgeIdx];
    double residual = edge.capacity - edge.flow;
    double pushAmount = qMin(m_excess[v], residual);

    if (pushAmount <= 0) return;

    edge.flow += pushAmount;
    m_adj[edge.to][edge.rev].flow -= pushAmount;
    m_excess[v] -= pushAmount;
    m_excess[edge.to] += pushAmount;

    ++m_stats.totalPushes;
}

/** @brief Relabel操作 @param v 顶点 */
void MaxFlowPushRelabel::relabel(int v)
{
    int minHeight = 2 * m_n + 1;
    for (const auto& edge : m_adj[v]) {
        if (edge.capacity - edge.flow > 0) {
            minHeight = qMin(minHeight, m_height[edge.to]);
        }
    }
    m_height[v] = minHeight + 1;
    ++m_stats.totalRelabels;
}

/** @brief Discharge操作 @param v 顶点 */
void MaxFlowPushRelabel::discharge(int v)
{
    while (m_excess[v] > 0) {
        if (m_current[v] < m_adj[v].size()) {
            auto& edge = m_adj[v][m_current[v]];
            if (edge.capacity - edge.flow > 0 && m_height[v] == m_height[edge.to] + 1) {
                push(v, m_current[v]);
            } else {
                ++m_current[v];
            }
        } else {
            /* 检查gap启发式 */
            gapHeuristic(m_height[v]);
            relabel(v);
            m_current[v] = 0;
        }
    }
}

/** @brief Gap启发式 @param gapHeight 间隔高度 */
void MaxFlowPushRelabel::gapHeuristic(int gapHeight)
{
    if (gapHeight <= 0 || gapHeight >= m_n) return;

    /* 检查是否存在该高度的顶点 */
    bool hasVertex = false;
    for (int v = 0; v < m_n; ++v) {
        if (v != m_source && v != m_sink && m_height[v] == gapHeight) {
            hasVertex = true;
            break;
        }
    }

    if (!hasVertex) return;

    /* 将所有高度>gapHeight的顶点提升到2n+1 */
    for (int v = 0; v < m_n; ++v) {
        if (v != m_source && v != m_sink && m_height[v] > gapHeight) {
            m_height[v] = qMax(m_height[v], 2 * m_n + 1);
        }
    }

    ++m_stats.totalGapRelabels;
    emit gapHeuristicTriggered(gapHeight);
}

/** @brief 全局重标号 @param source 源点 @param sink 汇点 */
void MaxFlowPushRelabel::globalRelabel(int source, int sink)
{
    /* 从汇点BFS计算精确距离 */
    QVector<int> dist(m_n, 2 * m_n + 1);
    dist[sink] = 0;

    std::queue<int> q;
    q.push(sink);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const auto& edge : m_adj[u]) {
            /* 沿反向边遍历(残余图中的反向边) */
            if (m_adj[edge.to][edge.rev].capacity - m_adj[edge.to][edge.rev].flow > 0) {
                if (dist[edge.to] > dist[u] + 1) {
                    dist[edge.to] = dist[u] + 1;
                    q.push(edge.to);
                }
            }
        }
    }

    /* 更新高度 */
    for (int v = 0; v < m_n; ++v) {
        if (v != source) {
            m_height[v] = dist[v];
        }
    }

    ++m_stats.totalGlobalRelabels;
}

/** @brief 判断边是否可允许 @param v 顶点 @param edgeIdx 边索引 */
bool MaxFlowPushRelabel::isAdmissible(int v, int edgeIdx) const
{
    const auto& edge = m_adj[v][edgeIdx];
    return edge.capacity - edge.flow > 0 && m_height[v] == m_height[edge.to] + 1;
}

/** @brief 计算最大流(基础Push-Relabel) @param source 源点 @param sink 汇点 @return 最大流值 */
double MaxFlowPushRelabel::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    m_source = source;
    m_sink = sink;

    initPreflow(source);

    /* 使用队列处理活跃顶点 */
    std::queue<int> queue;
    for (int i = 0; i < m_adj[source].size(); ++i) {
        int v = m_adj[source][i].to;
        if (v != sink && m_excess[v] > 0 && !m_inQueue[v]) {
            queue.push(v);
            m_inQueue[v] = true;
        }
    }

    while (!queue.empty()) {
        int v = queue.front();
        queue.pop();
        m_inQueue[v] = false;

        discharge(v);

        /* 将新的活跃顶点加入队列 */
        if (m_excess[v] > 0) {
            queue.push(v);
            m_inQueue[v] = true;
        }

        for (const auto& edge : m_adj[v]) {
            if (edge.to != source && edge.to != sink
                && m_excess[edge.to] > 0 && !m_inQueue[edge.to]) {
                queue.push(edge.to);
                m_inQueue[edge.to] = true;
            }
        }
    }

    double flow = m_excess[sink];
    m_stats.lastMaxFlow = flow;
    m_stats.lastVertexCount = m_n;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalGlobalRelabels + 1);

    emit maxFlowComputed(flow, m_stats.totalPushes, m_stats.totalRelabels);
    return flow;
}

/** @brief 最高标号选择策略 @param source 源点 @param sink 汇点 @return 最大流值 */
double MaxFlowPushRelabel::maxFlowHighestLabel(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    m_source = source;
    m_sink = sink;

    initPreflow(source);

    /* 按高度分桶(最高标号优先) */
    QMap<int, QVector<int>> buckets;
    for (int i = 0; i < m_adj[source].size(); ++i) {
        int v = m_adj[source][i].to;
        if (v != sink && m_excess[v] > 0) {
            buckets[m_height[v]].append(v);
        }
    }

    while (!buckets.isEmpty()) {
        /* 取最高标号的桶 */
        auto it = buckets.end(); --it;
        int h = it.key();
        auto& bucket = it.value();

        if (bucket.isEmpty()) {
            buckets.remove(h);
            continue;
        }

        int v = bucket.takeLast();

        discharge(v);

        /* 重新放入对应桶 */
        if (m_excess[v] > 0 && v != source && v != sink) {
            buckets[m_height[v]].append(v);
        }

        /* 检查新活跃顶点 */
        for (const auto& edge : m_adj[v]) {
            if (edge.to != source && edge.to != sink
                && m_excess[edge.to] > 0 && edge.flow > 0) {
                buckets[m_height[edge.to]].append(edge.to);
            }
        }

        /* 清理空桶 */
        for (auto bIt = buckets.begin(); bIt != buckets.end();) {
            if (bIt->isEmpty()) bIt = buckets.erase(bIt);
            else ++bIt;
        }
    }

    double flow = m_excess[sink];
    m_stats.lastMaxFlow = flow;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalGlobalRelabels + 1);

    emit maxFlowComputed(flow, m_stats.totalPushes, m_stats.totalRelabels);
    return flow;
}

/** @brief 带全局重标号的最大流 @param source 源点 @param sink 汇点 @param relabelInterval 间隔 @return 最大流值 */
double MaxFlowPushRelabel::maxFlowWithGlobalRelabel(int source, int sink,
                                                     int relabelInterval)
{
    QElapsedTimer timer;
    timer.start();

    m_source = source;
    m_sink = sink;
    quint64 pushCount = m_stats.totalPushes;

    initPreflow(source);
    globalRelabel(source, sink);

    std::queue<int> queue;
    for (int i = 0; i < m_adj[source].size(); ++i) {
        int v = m_adj[source][i].to;
        if (v != sink && m_excess[v] > 0) queue.push(v);
    }

    int iterCount = 0;
    while (!queue.empty()) {
        int v = queue.front();
        queue.pop();

        discharge(v);

        if (m_excess[v] > 0 && v != source && v != sink) {
            queue.push(v);
        }
        for (const auto& edge : m_adj[v]) {
            if (edge.to != source && edge.to != sink && m_excess[edge.to] > 0) {
                queue.push(edge.to);
            }
        }

        /* 定期全局重标号 */
        if (++iterCount % relabelInterval == 0) {
            globalRelabel(source, sink);
        }
    }

    double flow = m_excess[sink];
    m_stats.lastMaxFlow = flow;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalGlobalRelabels);

    emit maxFlowComputed(flow, m_stats.totalPushes - pushCount,
                         m_stats.totalRelabels);
    return flow;
}

/** @brief 获取最小割 @param source 源点 @return (S集合, T集合) */
QPair<QVector<int>, QVector<int>> MaxFlowPushRelabel::minCut(int source) const
{
    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    /* BFS沿残余图遍历可达顶点 */
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const auto& edge : m_adj[u]) {
            if (!visited[edge.to] && edge.capacity - edge.flow > 0) {
                visited[edge.to] = true;
                q.push(edge.to);
            }
        }
    }

    QVector<int> sSet, tSet;
    for (int v = 0; v < m_n; ++v) {
        if (visited[v]) sSet.append(v);
        else tSet.append(v);
    }

    return {sSet, tSet};
}

/** @brief 获取边流量 @return 所有边的信息 */
QVector<MaxFlowPushRelabel::Edge> MaxFlowPushRelabel::edges() const
{
    QVector<Edge> result;
    for (const auto& list : m_adj) {
        for (const auto& edge : list) {
            if (edge.capacity > 0) result.append(edge);
        }
    }
    return result;
}

/** @brief 获取指定顶点的邻接边 @param v 顶点 @return 边列表 */
QVector<MaxFlowPushRelabel::Edge> MaxFlowPushRelabel::vertexEdges(int v) const
{
    if (v < 0 || v >= m_n) return {};
    return m_adj[v];
}

/** @brief 重置统计 */
void MaxFlowPushRelabel::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
