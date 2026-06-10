/**
 * @file MaximumFlow7.cpp
 * @brief MaximumFlow7 实现
 *
 * 实现最大流：Dinic层次图BFS与阻塞流DFS分层网络增广。
 */

#include "utils/graph287/MaximumFlow7.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumFlow7::MaximumFlow7(QObject *parent)
    : QObject(parent) {}

MaximumFlow7::~MaximumFlow7() = default;

/* ---- Configuration ---- */

void MaximumFlow7::setNodes(int n)
{
    m_n = qBound(2, n, 100000);
    m_adj.resize(m_n);
}

/* ---- Add edge with forward+reverse ---- */

void MaximumFlow7::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;

    Edge fwd{to, m_adj[to].size(), capacity, 0.0};
    Edge rev{from, m_adj[from].size(), 0.0, 0.0};
    m_adj[from].append(fwd);
    m_adj[to].append(rev);
}

/* ---- BFS: build level graph ---- */

bool MaximumFlow7::buildLevelGraph(int source, int sink)
{
    m_level.fill(-1, m_n);
    m_level[source] = 0;

    QQueue<int> queue;
    queue.enqueue(source);

    while (!queue.isEmpty()) {
        int node = queue.dequeue();
        for (const Edge& e : m_adj[node]) {
            // Only traverse edges with remaining capacity
            if (m_level[e.to] < 0 && e.cap - e.flow > 1e-12) {
                m_level[e.to] = m_level[node] + 1;
                queue.enqueue(e.to);
            }
        }
    }
    return m_level[sink] >= 0;  // True if sink is reachable
}

/* ---- DFS: blocking flow with iterator optimization ---- */

double MaximumFlow7::blockingFlow(int node, int sink, double pushed)
{
    if (node == sink) return pushed;

    for (int& i = m_iter[node]; i < m_adj[node].size(); ++i) {
        Edge& e = m_adj[node][i];
        if (m_level[e.to] != m_level[node] + 1) continue;
        double remaining = e.cap - e.flow;
        if (remaining < 1e-12) continue;

        double flow = blockingFlow(e.to, sink, qMin(pushed, remaining));
        if (flow < 1e-12) continue;

        // Augment forward and reverse edges
        e.flow += flow;
        m_adj[e.to][e.rev].flow -= flow;
        return flow;
    }
    return 0.0;
}

/* ---- Reset edges for new computation ---- */

void MaximumFlow7::resetEdges()
{
    for (auto& edges : m_adj)
        for (auto& e : edges)
            e.flow = 0.0;
}

/* ---- Main max flow (Dinic's algorithm) ---- */

double MaximumFlow7::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    resetEdges();
    double totalFlow = 0.0;
    int phases = 0;

    while (buildLevelGraph(source, sink)) {
        m_iter.fill(0, m_n);
        double flow;
        while ((flow = blockingFlow(source, sink, 1e18)) > 1e-12) {
            totalFlow += flow;
        }
        phases++;
    }

    // Count edges
    int edgeCount = 0;
    for (const auto& edges : m_adj)
        edgeCount += edges.size();
    edgeCount /= 2;  // Forward + reverse pairs

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_n;
    m_stats.numEdges = edgeCount;
    m_stats.numBFSPhases = phases;
    m_stats.maxFlow = totalFlow;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit flowComputed(totalFlow, phases, elapsed);

    return totalFlow;
}

/* ---- Get flow on edge ---- */

double MaximumFlow7::edgeFlow(int edgeIndex) const
{
    int count = 0;
    for (int u = 0; u < m_adj.size(); ++u) {
        for (int i = 0; i < m_adj[u].size(); ++i) {
            // Only count forward edges (cap > 0 originally)
            if (m_adj[u][i].cap > 0) {
                if (count == edgeIndex) return m_adj[u][i].flow;
                count++;
            }
        }
    }
    return 0.0;
}

/* ---- Min-cut: BFS on residual graph ---- */

QVector<int> MaximumFlow7::minCut(int source) const
{
    QVector<int> visited(m_n, 0);
    QQueue<int> queue;
    queue.enqueue(source);
    visited[source] = 1;
    QVector<int> cut;

    while (!queue.isEmpty()) {
        int node = queue.dequeue();
        cut.append(node);
        for (const Edge& e : m_adj[node]) {
            if (!visited[e.to] && e.cap - e.flow > 1e-12) {
                visited[e.to] = 1;
                queue.enqueue(e.to);
            }
        }
    }
    return cut;
}

/* ---- Reset ---- */

void MaximumFlow7::resetStatistics()
{
    m_adj.clear();
    m_level.clear();
    m_iter.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
