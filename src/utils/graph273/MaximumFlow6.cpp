/**
 * @file MaximumFlow6.cpp
 * @brief MaximumFlow6 实现
 *
 * 实现最大流：Push-Relabel算法与间隙启发式全局重标号。
 */

#include "utils/graph273/MaximumFlow6.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>
#include <queue>

/* ---- Construction / Destruction ---- */

MaximumFlow6::MaximumFlow6(QObject *parent)
    : QObject(parent) {}
MaximumFlow6::~MaximumFlow6() = default;

/* ---- Initialize network ---- */

void MaximumFlow6::initNetwork(int numNodes)
{
    m_n = numNodes;
    m_graph.clear();
    m_graph.resize(m_n);
}

/* ---- Add edge ---- */

void MaximumFlow6::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;
    // Forward edge
    Edge fwd{to, m_graph[to].size(), capacity, 0.0};
    // Reverse edge
    Edge rev{from, m_graph[from].size(), 0.0, 0.0};
    m_graph[from].append(fwd);
    m_graph[to].append(rev);
    m_stats.numEdges += 2;
}

/* ---- Push flow ---- */

void MaximumFlow6::push(int u, int ei)
{
    Edge& e = m_graph[u][ei];
    double flow = qMin(m_excess[u], e.capacity - e.flow);
    if (flow <= 0.0) return;

    e.flow += flow;
    m_graph[e.to][e.rev].flow -= flow;
    m_excess[u] -= flow;
    m_excess[e.to] += flow;
    m_stats.numPushes++;
}

/* ---- Relabel ---- */

void MaximumFlow6::relabel(int u)
{
    int minH = std::numeric_limits<int>::max();
    for (const Edge& e : m_graph[u]) {
        if (e.capacity - e.flow > 0.0 && m_height[e.to] < minH)
            minH = m_height[e.to];
    }
    if (minH < std::numeric_limits<int>::max()) {
        m_count[m_height[u]]--;
        m_height[u] = minH + 1;
        m_count[m_height[u]]++;
        m_stats.numRelabels++;
    }
}

/* ---- Gap heuristic ---- */

void MaximumFlow6::gapHeuristic(int gapLevel)
{
    for (int u = 0; u < m_n; ++u) {
        if (m_height[u] > gapLevel && m_height[u] < m_n) {
            m_count[m_height[u]]--;
            m_height[u] = m_n + 1;  // Effectively infinite
            m_count[m_height[u]]++;
            m_stats.numGaps++;
        }
    }
}

/* ---- Global relabeling via reverse BFS ---- */

void MaximumFlow6::globalRelabel(int source, int sink)
{
    QVector<int> dist(m_n, 2 * m_n);
    dist[sink] = 0;
    QVector<bool> visited(m_n, false);
    QQueue<int> queue;
    queue.enqueue(sink);
    visited[sink] = true;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (const Edge& e : m_graph[u]) {
            // Check reverse edge capacity
            int v = e.to;
            const Edge& revEdge = m_graph[v][e.rev];
            if (!visited[v] && revEdge.capacity - revEdge.flow > 0.0) {
                visited[v] = true;
                dist[v] = dist[u] + 1;
                queue.enqueue(v);
            }
        }
    }

    // Update heights
    m_count.clear();
    m_count.resize(2 * m_n + 1, 0);
    for (int u = 0; u < m_n; ++u) {
        m_height[u] = dist[u];
        m_count[m_height[u]]++;
    }
    m_height[source] = qMax(m_height[source], m_n);
}

/* ---- Discharge node ---- */

void MaximumFlow6::discharge(int u)
{
    int ei = 0;
    while (m_excess[u] > 0.0) {
        if (ei >= m_graph[u].size()) {
            // Check gap heuristic before relabeling
            int h = m_height[u];
            if (h < m_n && h >= 0 && h < m_count.size() && m_count[h] == 0)
                gapHeuristic(h);
            relabel(u);
            ei = 0;
        } else {
            const Edge& e = m_graph[u][ei];
            if (e.capacity - e.flow > 0.0 && m_height[u] == m_height[e.to] + 1)
                push(u, ei);
            else
                ei++;
        }
    }
}

/* ---- Max flow ---- */

double MaximumFlow6::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    m_excess.resize(m_n, 0.0);
    m_height.resize(m_n, 0);
    m_count.resize(2 * m_n + 1, 0);
    m_excess.fill(0.0);
    m_height.fill(0);
    m_count.fill(0);

    // Initialize
    m_height[source] = m_n;
    m_count[0] = m_n - 1;
    m_count[m_n] = 1;

    // Saturate edges from source
    for (int i = 0; i < m_graph[source].size(); ++i) {
        Edge& e = m_graph[source][i];
        double flow = e.capacity;
        e.flow = flow;
        m_graph[e.to][e.rev].flow = -flow;
        m_excess[e.to] += flow;
    }

    // Global relabel for initial heights
    globalRelabel(source, sink);

    // Discharge loop
    QVector<int> list;
    for (int u = 0; u < m_n; ++u)
        if (u != source && u != sink) list.append(u);

    int idx = 0;
    int globalRelabelCount = 0;
    while (idx < list.size()) {
        int u = list[idx];
        int oldHeight = m_height[u];
        discharge(u);
        if (m_height[u] > oldHeight) {
            // Move to front
            list.removeAt(idx);
            list.prepend(u);
            idx = 0;
        }
        idx++;

        // Periodic global relabeling
        globalRelabelCount++;
        if (globalRelabelCount >= m_n) {
            globalRelabel(source, sink);
            globalRelabelCount = 0;
        }
    }

    double flow = m_excess[sink];

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_n;
    m_stats.maxFlowValue = flow;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit flowComputed(flow, m_stats.numPushes, m_stats.numRelabels, elapsed);
    return flow;
}

/* ---- Min cut ---- */

QVector<int> MaximumFlow6::minCut(int source) const
{
    QVector<bool> visited(m_n, false);
    QQueue<int> queue;
    queue.enqueue(source);
    visited[source] = true;
    QVector<int> result;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        result.append(u);
        for (const Edge& e : m_graph[u]) {
            if (!visited[e.to] && e.capacity - e.flow > 1e-12) {
                visited[e.to] = true;
                queue.enqueue(e.to);
            }
        }
    }
    return result;
}

/* ---- Edge flows ---- */

QVector<QPair<int, int>> MaximumFlow6::edgeFlows() const
{
    QVector<QPair<int, int>> result;
    for (int u = 0; u < m_n; ++u) {
        for (const Edge& e : m_graph[u]) {
            if (e.flow > 0.0)
                result.append({u, e.to});
        }
    }
    return result;
}

/* ---- Residual capacity ---- */

double MaximumFlow6::residualCapacity(int from, int edgeIdx) const
{
    if (from < 0 || from >= m_n || edgeIdx >= m_graph[from].size()) return 0.0;
    return m_graph[from][edgeIdx].capacity - m_graph[from][edgeIdx].flow;
}

/* ---- Reset ---- */

void MaximumFlow6::resetStatistics()
{
    m_graph.clear();
    m_excess.clear();
    m_height.clear();
    m_count.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
