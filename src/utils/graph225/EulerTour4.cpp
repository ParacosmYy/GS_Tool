/**
 * @file EulerTour4.cpp
 * @brief EulerTour4 实现
 *
 * 实现欧拉路径：Hierholzer遍历、多重图边着色、路径分解。
 */

#include "utils/graph225/EulerTour4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EulerTour4::EulerTour4(QObject *parent) : QObject(parent) {}
EulerTour4::~EulerTour4() = default;

/* ---- Configuration ---- */

void EulerTour4::setDirected(bool directed) { m_directed = directed; }

/* ---- Add edge ---- */

void EulerTour4::addEdge(int u, int v, int count)
{
    count = qMax(1, count);
    for (int i = 0; i < count; ++i) {
        int edgeId = m_edges.size();
        m_edges.append({u, v});
        // Ensure adjacency list is large enough
        int maxV = qMax(u, v) + 1;
        if (maxV > m_adj.size()) {
            m_adj.resize(maxV);
            m_numVertices = maxV;
        }
        m_adj[u].append({v, edgeId});
        if (!m_directed)
            m_adj[v].append({u, edgeId});
    }
    m_numVertices = qMax(m_numVertices, qMax(u, v) + 1);
}

/* ---- Build from adjacency ---- */

void EulerTour4::buildGraph(int numVertices,
                             const QVector<QVector<QPair<int, int>>>& adj)
{
    m_numVertices = numVertices;
    m_adj = adj;
    m_edges.clear();
    if (m_adj.size() < numVertices) m_adj.resize(numVertices);
}

/* ---- Compute degrees ---- */

void EulerTour4::computeDegrees(QVector<int>& inDeg,
                                 QVector<int>& outDeg) const
{
    inDeg.resize(m_numVertices, 0);
    outDeg.resize(m_numVertices, 0);
    for (const auto& edge : m_edges) {
        outDeg[edge.first]++;
        inDeg[edge.second]++;
        if (!m_directed) {
            inDeg[edge.first]++;
            outDeg[edge.second]++;
        }
    }
}

/* ---- Has Euler circuit ---- */

bool EulerTour4::hasEulerianCircuit() const
{
    if (m_edges.isEmpty()) return false;

    QVector<int> inDeg, outDeg;
    computeDegrees(inDeg, outDeg);

    for (int v = 0; v < m_numVertices; ++v) {
        // Check connectivity implicitly via degrees
        if (m_directed) {
            if (inDeg[v] != outDeg[v]) return false;
        } else {
            if ((inDeg[v] + outDeg[v]) % 2 != 0) return false;
        }
    }
    return true;
}

/* ---- Has Euler path ---- */

bool EulerTour4::hasEulerianPath() const
{
    if (m_edges.isEmpty()) return false;

    QVector<int> inDeg, outDeg;
    computeDegrees(inDeg, outDeg);

    if (m_directed) {
        int startCandidates = 0, endCandidates = 0;
        for (int v = 0; v < m_numVertices; ++v) {
            int diff = outDeg[v] - inDeg[v];
            if (diff == 1) startCandidates++;
            else if (diff == -1) endCandidates++;
            else if (diff != 0) return false;
        }
        return startCandidates <= 1 && endCandidates <= 1;
    }

    // Undirected: exactly 0 or 2 vertices with odd degree
    int oddCount = 0;
    for (int v = 0; v < m_numVertices; ++v) {
        if ((inDeg[v] + outDeg[v]) % 2 != 0) oddCount++;
    }
    return oddCount == 0 || oddCount == 2;
}

/* ---- Find start vertex ---- */

int EulerTour4::findStartVertex() const
{
    QVector<int> inDeg, outDeg;
    computeDegrees(inDeg, outDeg);

    if (m_directed) {
        for (int v = 0; v < m_numVertices; ++v) {
            if (outDeg[v] - inDeg[v] == 1) return v;
        }
    } else {
        for (int v = 0; v < m_numVertices; ++v) {
            if ((inDeg[v] + outDeg[v]) % 2 != 0) return v;
        }
    }
    // Default: first vertex with edges
    for (int v = 0; v < m_numVertices; ++v) {
        if (!m_adj[v].isEmpty()) return v;
    }
    return 0;
}

/* ---- Hierholzer ---- */

QVector<int> EulerTour4::hierholzer(int start)
{
    int n = m_edges.size();
    m_edgeUsed.resize(n, false);

    // Copy adjacency with mutable index pointers
    QVector<int> adjIdx(m_numVertices, 0);
    QVector<int> stack;
    QVector<int> circuit;

    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;

        while (adjIdx[v] < m_adj[v].size()) {
            int ei = adjIdx[v];
            int neighbor = m_adj[v][ei].first;
            int edgeId = m_adj[v][ei].second;
            adjIdx[v]++;

            if (!m_edgeUsed[edgeId]) {
                m_edgeUsed[edgeId] = true;
                stack.append(neighbor);
                found = true;
                break;
            }
        }

        if (!found) {
            circuit.append(stack.back());
            stack.removeLast();
        }
    }

    // Reverse to get correct order
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

/* ---- Find Euler circuit ---- */

QVector<int> EulerTour4::findEulerCircuit()
{
    QElapsedTimer timer;
    timer.start();

    if (m_edges.isEmpty()) return {};
    if (!hasEulerianCircuit()) return {};

    int start = 0;
    for (int v = 0; v < m_numVertices; ++v) {
        if (!m_adj[v].isEmpty()) { start = v; break; }
    }

    auto circuit = hierholzer(start);

    m_stats.numEdges = m_edges.size();
    m_stats.numVertices = m_numVertices;
    m_stats.hasEulerCircuit = true;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourFound(circuit, timer.elapsed());

    return circuit;
}

/* ---- Find Euler path ---- */

QVector<int> EulerTour4::findEulerPath()
{
    QElapsedTimer timer;
    timer.start();

    if (m_edges.isEmpty()) return {};
    if (!hasEulerianPath()) return {};

    int start = findStartVertex();
    auto path = hierholzer(start);

    m_stats.numEdges = m_edges.size();
    m_stats.numVertices = m_numVertices;
    m_stats.hasEulerPath = true;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourFound(path, timer.elapsed());

    return path;
}

/* ---- Decompose into trails ---- */

QVector<QVector<int>> EulerTour4::decomposeTrails()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> trails;
    QVector<int> inDeg, outDeg;
    computeDegrees(inDeg, outDeg);

    // Reset edge usage
    m_edgeUsed.resize(m_edges.size(), false);
    m_edgeUsed.fill(false);

    int remainingEdges = m_edges.size();
    while (remainingEdges > 0) {
        // Find start vertex for next trail
        int start = 0;
        for (int v = 0; v < m_numVertices; ++v) {
            for (const auto& pr : m_adj[v]) {
                if (!m_edgeUsed[pr.second]) { start = v; break; }
            }
        }

        // Build single trail
        QVector<int> trail;
        trail.append(start);
        int current = start;
        bool advanced = true;

        while (advanced) {
            advanced = false;
            for (int i = 0; i < m_adj[current].size(); ++i) {
                int edgeId = m_adj[current][i].second;
                if (!m_edgeUsed[edgeId]) {
                    m_edgeUsed[edgeId] = true;
                    current = m_adj[current][i].first;
                    trail.append(current);
                    remainingEdges--;
                    advanced = true;
                    break;
                }
            }
        }

        trails.append(trail);
    }

    m_stats.numTrails = trails.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return trails;
}

/* ---- Edge coloring ---- */

QVector<QVector<QPair<int, int>>> EulerTour4::edgeColoring() const
{
    // Color parallel edges with different colors for trail decomposition
    // Uses greedy coloring per vertex pair
    int maxColor = 0;
    QVector<int> color(m_edges.size(), -1);

    // Group edges by (u,v) pair
    QMap<QPair<int, int>, QVector<int>> edgeGroups;
    for (int i = 0; i < m_edges.size(); ++i) {
        auto key = m_edges[i];
        if (!m_directed && key.first > key.second)
            key = {key.second, key.first};
        edgeGroups[key].append(i);
    }

    // Assign colors within each group
    for (auto it = edgeGroups.begin(); it != edgeGroups.end(); ++it) {
        const auto& indices = it.value();
        for (int c = 0; c < indices.size(); ++c) {
            color[indices[c]] = c;
            maxColor = qMax(maxColor, c + 1);
        }
    }

    // Group by color
    QVector<QVector<QPair<int, int>>> result(maxColor);
    for (int i = 0; i < m_edges.size(); ++i) {
        int c = qMax(0, color[i]);
        if (c < maxColor)
            result[c].append(m_edges[i]);
    }

    return result;
}

/* ---- Reset ---- */

void EulerTour4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_edges.clear();
    m_edgeUsed.clear();
    m_numVertices = 0;
}
