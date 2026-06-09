/**
 * @file EulerTour7.cpp
 * @brief EulerTour7 实现
 *
 * 实现欧拉回路：Fleury割边检测与边栈Hierholzer高效路径构建。
 */

#include "utils/graph267/EulerTour7.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EulerTour7::EulerTour7(QObject *parent) : QObject(parent) {}
EulerTour7::~EulerTour7() = default;

/* ---- Build graph from edge list ---- */

void EulerTour7::buildGraph(int numVertices, const QVector<QPair<int,int>>& edges)
{
    m_n = numVertices;
    m_edgeCount = edges.size();
    m_adj.resize(m_n);
    m_edgeUsed.resize(m_edgeCount * 2, false);

    for (int i = 0; i < m_n; ++i) m_adj[i].clear();

    // Add undirected edges
    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].first;
        int v = edges[i].second;
        // Forward edge
        Edge e1;
        e1.to = v;
        e1.id = i * 2;
        e1.used = false;
        // Reverse edge
        Edge e2;
        e2.to = u;
        e2.id = i * 2 + 1;
        e2.used = false;

        m_adj[u].append(e1);
        m_adj[v].append(e2);
    }
}

/* ---- Count reachable vertices via DFS ---- */

int EulerTour7::countReachable(int v, QVector<bool>& visited) const
{
    visited[v] = true;
    int count = 1;
    for (const auto& e : m_adj[v]) {
        if (!e.used && !visited[e.to])
            count += countReachable(e.to, visited);
    }
    return count;
}

/* ---- Check if edge is a bridge (Fleury) ---- */

bool EulerTour7::isBridge(int u, int edgeIdx) const
{
    // Count reachable vertices before removing edge
    QVector<bool> visited(m_n, false);
    int before = countReachable(u, visited);

    // Temporarily mark edge as used
    const_cast<QVector<Edge>&>(m_adj[u])[edgeIdx].used = true;
    // Also mark reverse edge
    int target = m_adj[u][edgeIdx].to;
    for (auto& e : const_cast<QVector<QVector<Edge>>&>(m_adj)[target]) {
        if (e.id == (m_adj[u][edgeIdx].id ^ 1)) {
            e.used = true;
            break;
        }
    }

    visited.fill(false);
    int after = countReachable(u, visited);

    // Restore edges
    const_cast<QVector<Edge>&>(m_adj[u])[edgeIdx].used = false;
    for (auto& e : const_cast<QVector<QVector<Edge>>&>(m_adj)[target]) {
        if (e.id == (m_adj[u][edgeIdx].id ^ 1)) {
            e.used = false;
            break;
        }
    }

    m_stats.numBridgeChecks++;
    return (after < before);
}

/* ---- Count connected components ---- */

int EulerTour7::countComponents() const
{
    QVector<bool> visited(m_n, false);
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        if (!visited[i]) {
            countReachable(i, visited);
            count++;
        }
    }
    return count;
}

/* ---- Check for Eulerian circuit ---- */

bool EulerTour7::hasEulerCircuit() const
{
    // All vertices must have even degree
    for (int i = 0; i < m_n; ++i) {
        int deg = 0;
        for (const auto& e : m_adj[i])
            if (!e.used) deg++;
        if (deg % 2 != 0) return false;
    }
    return countComponents() == 1;
}

/* ---- Check for Eulerian path ---- */

bool EulerTour7::hasEulerPath() const
{
    int oddCount = 0;
    for (int i = 0; i < m_n; ++i) {
        int deg = 0;
        for (const auto& e : m_adj[i])
            if (!e.used) deg++;
        if (deg % 2 != 0) oddCount++;
    }
    return oddCount == 2 || oddCount == 0;
}

/* ---- Find valid start vertex ---- */

int EulerTour7::findStartVertex() const
{
    // If Eulerian path exists, start from odd-degree vertex
    for (int i = 0; i < m_n; ++i) {
        int deg = 0;
        for (const auto& e : m_adj[i])
            if (!e.used) deg++;
        if (deg % 2 != 0) return i;
    }
    return 0; // Eulerian circuit: start anywhere
}

/* ---- Find Euler tour using edge-stack Hierholzer ---- */

QVector<int> EulerTour7::findEulerTour()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || !hasEulerPath()) return {};

    int start = findStartVertex();
    QVector<int> tour;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.last();
        bool found = false;

        // Find an unused non-bridge edge (Fleury heuristic)
        int bestEdge = -1;
        bool hasNonBridge = false;

        for (int i = 0; i < m_adj[v].size(); ++i) {
            if (!m_adj[v][i].used) {
                if (!isBridge(v, i)) {
                    bestEdge = i;
                    hasNonBridge = true;
                    break;
                }
                // Remember a bridge edge as fallback
                bestEdge = i;
            }
        }

        if (bestEdge >= 0) {
            Edge& e = m_adj[v][bestEdge];
            e.used = true;
            // Mark reverse edge used
            for (auto& re : m_adj[e.to]) {
                if (re.id == (e.id ^ 1)) {
                    re.used = true;
                    break;
                }
            }
            stack.append(e.to);
            found = true;
        }

        if (!found) {
            // No more edges from v: add to tour
            tour.append(v);
            stack.removeLast();
        }
    }

    // Tour is in reverse order, reverse it
    std::reverse(tour.begin(), tour.end());

    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCount;
    m_stats.tourLength = tour.size();
    m_stats.numComponents = countComponents();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourCompleted(tour.size(), timer.elapsed());
    return tour;
}

/* ---- Reset ---- */

void EulerTour7::resetStatistics()
{
    m_adj.clear();
    m_edgeUsed.clear();
    m_n = 0;
    m_edgeCount = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
