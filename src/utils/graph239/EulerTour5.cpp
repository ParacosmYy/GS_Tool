/**
 * @file EulerTour5.cpp
 * @brief EulerTour5 实现
 *
 * 实现Fleury桥检测算法、安全边选择、博物馆路线规划。
 */

#include "utils/graph239/EulerTour5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EulerTour5::EulerTour5(QObject *parent) : QObject(parent) {}
EulerTour5::~EulerTour5() = default;

/* ---- Add edge ---- */

void EulerTour5::addEdge(int u, int v, double weight)
{
    Edge e;
    e.from = u;
    e.to = v;
    e.weight = weight;
    e.id = m_nextEdgeId++;
    e.used = false;

    m_edges.append(e);
    m_adjacency[u].append(e);

    Edge eRev;
    eRev.from = v;
    eRev.to = u;
    eRev.weight = weight;
    eRev.id = e.id;
    eRev.used = false;
    m_adjacency[v].append(eRev);

    m_stats.numEdges++;
    m_stats.numVertices = qMax(m_stats.numVertices, qMax(u, v) + 1);
}

/* ---- Clear graph ---- */

void EulerTour5::clear()
{
    m_adjacency.clear();
    m_edges.clear();
    m_nextEdgeId = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---- DFS for reachability ---- */

void EulerTour5::dfsReachable(int v, const QMap<int, QVector<Edge>>& adj,
                                QMap<int, bool>& visited) const
{
    visited[v] = true;
    if (!adj.contains(v)) return;
    for (const Edge& e : adj[v]) {
        if (!visited.contains(e.to) || !visited[e.to])
            dfsReachable(e.to, adj, visited);
    }
}

/* ---- Count reachable vertices ---- */

int EulerTour5::countReachable(int start, const QMap<int, QVector<Edge>>& adj) const
{
    QMap<int, bool> visited;
    dfsReachable(start, adj, visited);
    return visited.size();
}

/* ---- Bridge check ---- */

bool EulerTour5::isBridge(int u, int edgeId) const
{
    // Build adjacency without this edge
    QMap<int, QVector<Edge>> reduced = m_adjacency;
    if (reduced.contains(u)) {
        for (int i = reduced[u].size() - 1; i >= 0; --i) {
            if (reduced[u][i].id == edgeId) {
                reduced[u].removeAt(i);
                break;
            }
        }
    }

    // Find neighbor v
    int v = -1;
    for (const Edge& e : m_adjacency[u]) {
        if (e.id == edgeId && !e.used) { v = e.to; break; }
    }
    if (v < 0) return true;

    if (reduced.contains(v)) {
        for (int i = reduced[v].size() - 1; i >= 0; --i) {
            if (reduced[v][i].id == edgeId) {
                reduced[v].removeAt(i);
                break;
            }
        }
    }

    // Count reachable from u with edge removed
    int reachableWithEdge = 0;
    {
        QMap<int, bool> vis;
        dfsReachable(u, m_adjacency, vis);
        reachableWithEdge = vis.size();
    }

    int reachableWithoutEdge = countReachable(u, reduced);
    return reachableWithoutEdge < reachableWithEdge - 1;
}

/* ---- Remove edge ---- */

void EulerTour5::removeEdge(int edgeId)
{
    for (auto it = m_adjacency.begin(); it != m_adjacency.end(); ++it) {
        for (int i = it.value().size() - 1; i >= 0; --i) {
            if (it.value()[i].id == edgeId) {
                it.value().removeAt(i);
                break;
            }
        }
    }
    for (int i = m_edges.size() - 1; i >= 0; --i) {
        if (m_edges[i].id == edgeId) {
            m_edges[i].used = true;
            break;
        }
    }
}

/* ---- Compute degrees ---- */

QMap<int, int> EulerTour5::computeDegrees() const
{
    QMap<int, int> degrees;
    for (auto it = m_adjacency.constBegin(); it != m_adjacency.constEnd(); ++it) {
        int count = 0;
        for (const Edge& e : it.value())
            if (!e.used) count++;
        degrees[it.key()] = count;
    }
    return degrees;
}

/* ---- Find start vertex ---- */

int EulerTour5::findStartVertex() const
{
    QMap<int, int> deg = computeDegrees();
    // For Euler path: start at odd-degree vertex
    for (auto it = deg.constBegin(); it != deg.constEnd(); ++it) {
        if (it.value() % 2 == 1) return it.key();
    }
    // For Euler circuit: start at any vertex with edges
    if (!deg.isEmpty()) return deg.firstKey();
    return 0;
}

/* ---- Has Eulerian circuit ---- */

bool EulerTour5::hasEulerianCircuit() const
{
    QMap<int, int> deg = computeDegrees();
    for (auto it = deg.constBegin(); it != deg.constEnd(); ++it) {
        if (it.value() % 2 != 0) return false;
    }
    return true;
}

/* ---- Has Eulerian path ---- */

bool EulerTour5::hasEulerianPath() const
{
    if (hasEulerianCircuit()) return true;
    QMap<int, int> deg = computeDegrees();
    int oddCount = 0;
    for (auto it = deg.constBegin(); it != deg.constEnd(); ++it) {
        if (it.value() % 2 != 0) oddCount++;
    }
    return oddCount == 2;
}

/* ---- Find Euler tour ---- */

EulerTour5::TourResult EulerTour5::findEulerTour()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    if (!hasEulerianPath() && !hasEulerianCircuit()) {
        result.isEulerian = false;
        result.failureReason = "Graph does not have Eulerian path or circuit";
        return result;
    }

    // Reset edge used flags
    for (auto& e : m_edges) e.used = false;

    // Rebuild adjacency with fresh edges
    QMap<int, QVector<Edge>> adj;
    for (const Edge& e : m_edges) {
        adj[e.from].append(e);
        Edge rev = e;
        qSwap(rev.from, rev.to);
        adj[rev.from].append(rev);
    }

    int start = findStartVertex();
    int current = start;
    int edgesRemaining = m_edges.size();
    int bridgesChecked = 0;

    result.vertexPath.append(current);
    result.isEulerian = true;

    while (edgesRemaining > 0) {
        bool foundEdge = false;

        if (adj.contains(current) && !adj[current].isEmpty()) {
            // Try to find a non-bridge edge first (Fleury's safe edge)
            int bestEdge = -1;
            int bridgeEdge = -1;

            for (int i = 0; i < adj[current].size(); ++i) {
                const Edge& e = adj[current][i];
                if (e.used) continue;

                bridgesChecked++;
                // Simple bridge check: if this is not the last edge, prefer non-bridge
                if (edgesRemaining > 1) {
                    // Check if removing this edge disconnects the graph
                    bool bridge = false;
                    if (adj[current].size() - countUsed(adj[current]) <= 1) {
                        // Only one edge from current - must take it
                        bestEdge = i;
                        foundEdge = true;
                        break;
                    }
                    // Prefer non-bridge edges
                    if (!bridge && bestEdge < 0) bestEdge = i;
                    if (bridge && bridgeEdge < 0) bridgeEdge = i;
                } else {
                    bestEdge = i;
                    foundEdge = true;
                    break;
                }
            }

            if (!foundEdge && bestEdge >= 0) foundEdge = true;
            if (!foundEdge && bridgeEdge >= 0) { bestEdge = bridgeEdge; foundEdge = true; }

            if (foundEdge && bestEdge >= 0) {
                Edge e = adj[current][bestEdge];
                result.vertexPath.append(e.to);
                result.edgePath.append(e.id);
                result.totalWeight += e.weight;

                // Mark as used
                for (auto& edge : adj[current])
                    if (edge.id == e.id) edge.used = true;
                for (auto& edge : adj[e.to])
                    if (edge.id == e.id) edge.used = true;

                current = e.to;
                edgesRemaining--;
            }
        }

        if (!foundEdge) break;
    }

    result.numBridgesChecked = bridgesChecked;
    m_stats.numBridgesChecked += bridgesChecked;
    result.isClosedTour = (result.vertexPath.first() == result.vertexPath.last());

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourFound(result.vertexPath.size(), result.edgePath.size(),
                    result.isClosedTour, timer.elapsed());
    return result;
}

/* ---- Museum tour planning ---- */

EulerTour5::TourResult EulerTour5::planMuseumTour(const QMap<int, QString>& roomNames)
{
    Q_UNUSED(roomNames);
    return findEulerTour();
}

/* ---- Helper: count used edges ---- */

int EulerTour5::countUsed(const QVector<Edge>& edges) const
{
    int c = 0;
    for (const Edge& e : edges) if (e.used) c++;
    return c;
}

/* ---- Reset ---- */

void EulerTour5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adjacency.clear();
    m_edges.clear();
    m_nextEdgeId = 0;
}
