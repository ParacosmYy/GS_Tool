/**
 * @file MaximumFlow2.cpp
 * @brief MaximumFlow2 实现
 *
 * 实现最大流：Push-Relabel算法、间隙启发式、全局重标号优化。
 */

#include "utils/graph213/MaximumFlow2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <queue>

/* ---- Construction / Destruction ---- */

MaximumFlow2::MaximumFlow2(QObject *parent) : QObject(parent) {}
MaximumFlow2::~MaximumFlow2() = default;

/* ---- Configuration ---- */

void MaximumFlow2::setNumNodes(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/* ---- Add edge ---- */

void MaximumFlow2::addEdge(int u, int v, double capacity)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    int szU = m_adj[u].size();
    int szV = m_adj[v].size();
    m_adj[u].append({v, capacity, 0.0, szV});
    m_adj[v].append({u, 0.0, 0.0, szU});  // reverse edge
}

/* ---- Push ---- */

void MaximumFlow2::push(int u, int eIdx)
{
    auto& e = m_adj[u][eIdx];
    double d = qMin(m_excess[u], e.capacity - e.flow);
    if (d <= 0.0) return;

    e.flow += d;
    m_adj[e.to][e.rev].flow -= d;
    m_excess[u] -= d;
    m_excess[e.to] += d;
}

/* ---- Relabel ---- */

void MaximumFlow2::relabel(int u)
{
    int minH = std::numeric_limits<int>::max();
    for (const auto& e : m_adj[u]) {
        if (e.capacity - e.flow > 1e-12)
            minH = qMin(minH, m_height[e.to]);
    }
    if (minH < std::numeric_limits<int>::max())
        m_height[u] = minH + 1;
}

/* ---- Is admissible ---- */

bool MaximumFlow2::isAdmissible(int u, int eIdx) const
{
    const auto& e = m_adj[u][eIdx];
    return e.capacity - e.flow > 1e-12 && m_height[u] == m_height[e.to] + 1;
}

/* ---- Discharge ---- */

void MaximumFlow2::discharge(int u)
{
    while (m_excess[u] > 1e-12) {
        bool pushed = false;
        for (int i = 0; i < m_adj[u].size(); ++i) {
            if (isAdmissible(u, i)) {
                push(u, i);
                pushed = true;
                if (m_excess[u] <= 1e-12) break;
            }
        }
        if (!pushed) {
            int oldH = m_height[u];
            relabel(u);
            // Gap heuristic: check if we created a gap
            if (m_height[u] > oldH && oldH < m_n * 2) {
                bool gap = true;
                for (int i = 0; i < m_n; ++i) {
                    if (i != u && m_height[i] == oldH && m_excess[i] > 1e-12) {
                        gap = false; break;
                    }
                }
                if (gap) gapHeuristic(oldH);
            }
            if (m_height[u] > m_n * 2) break; // Safety
        }
    }
}

/* ---- Gap heuristic ---- */

void MaximumFlow2::gapHeuristic(int gapHeight)
{
    for (int i = 0; i < m_n; ++i) {
        if (m_height[i] > gapHeight && m_height[i] < m_n * 2)
            m_height[i] = m_n * 2 + 1;
    }
}

/* ---- Global relabeling ---- */

void MaximumFlow2::globalRelabel(int sink)
{
    m_height.fill(m_n * 2);
    m_height[sink] = 0;

    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(sink);
    visited[sink] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (const auto& e : m_adj[u]) {
            // Reverse edge has residual capacity
            const auto& revE = m_adj[e.to][e.rev];
            if (!visited[e.to] && revE.capacity - revE.flow > 1e-12) {
                visited[e.to] = true;
                m_height[e.to] = m_height[u] + 1;
                q.push(e.to);
            }
        }
    }
}

/* ---- Solve ---- */

double MaximumFlow2::solve(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (source < 0 || source >= m_n || sink < 0 || sink >= m_n) return 0.0;

    // Initialize
    m_excess.fill(0.0, m_n);
    m_height.fill(0, m_n);
    m_height[source] = m_n;

    // Initial saturating pushes from source
    for (int i = 0; i < m_adj[source].size(); ++i) {
        auto& e = m_adj[source][i];
        double d = e.capacity;
        if (d > 0.0) {
            e.flow = d;
            m_adj[e.to][e.rev].flow = -d;
            m_excess[e.to] += d;
        }
    }

    // Main loop with periodic global relabeling
    int relabelCount = 0;
    const int relabelInterval = m_n; // Global relabel every N relabels

    bool changed = true;
    while (changed) {
        changed = false;
        for (int u = 0; u < m_n; ++u) {
            if (u != source && u != sink && m_excess[u] > 1e-12) {
                discharge(u);
                changed = true;
            }
        }

        relabelCount++;
        if (relabelCount >= relabelInterval) {
            globalRelabel(sink);
            relabelCount = 0;
        }
    }

    double maxFlow = 0.0;
    for (const auto& e : m_adj[source])
        maxFlow += e.flow;

    m_stats.totalSolves++;
    m_stats.numNodes = m_n;
    int edgeCount = 0;
    for (const auto& adj : m_adj) edgeCount += adj.size();
    m_stats.numEdges = edgeCount / 2;
    m_stats.maxFlow = maxFlow;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_n, maxFlow, timer.elapsed());
    return maxFlow;
}

/* ---- Flow network ---- */

QVector<QVector<MaximumFlow2::Edge>> MaximumFlow2::flowNetwork() const
{
    return m_adj;
}

/* ---- Min cut ---- */

QVector<int> MaximumFlow2::minCut(int source) const
{
    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (const auto& e : m_adj[u]) {
            if (!visited[e.to] && e.capacity - e.flow > 1e-12) {
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }

    QVector<int> cut;
    for (int i = 0; i < m_n; ++i)
        if (visited[i]) cut.append(i);
    return cut;
}

/* ---- Inflow / Outflow ---- */

double MaximumFlow2::inflow(int node) const
{
    double flow = 0.0;
    for (const auto& adj : m_adj)
        for (const auto& e : adj)
            if (e.to == node && e.flow > 0.0) flow += e.flow;
    return flow;
}

double MaximumFlow2::outflow(int node) const
{
    double flow = 0.0;
    if (node >= 0 && node < m_adj.size())
        for (const auto& e : m_adj[node])
            if (e.flow > 0.0) flow += e.flow;
    return flow;
}

/* ---- Reset ---- */

void MaximumFlow2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_n = 0;
}
