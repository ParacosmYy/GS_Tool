/**
 * @file EulerTour10.cpp
 * @brief EulerTour10 实现
 *
 * 实现欧拉回路：Hierholzer电路遍历与Fleury避桥保证混合图中欧拉路径/回路。
 */

#include "utils/graph309/EulerTour10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <QStack>

/* ---- Construction / Destruction ---- */

EulerTour10::EulerTour10(QObject *parent)
    : QObject(parent) {}

EulerTour10::~EulerTour10() = default;

/* ---- Ensure vertex exists ---- */

void EulerTour10::ensureVertex(int v)
{
    if (v < 0) return;
    if (v >= m_numVertices) {
        m_adj.resize(v + 1);
        m_degree.resize(v + 1, 0);
        m_numVertices = v + 1;
    }
}

/* ---- Add undirected edge ---- */

void EulerTour10::addEdge(int u, int v)
{
    ensureVertex(u);
    ensureVertex(v);

    Edge e1{v, m_edgeCounter, false};
    Edge e2{u, m_edgeCounter, false};
    m_adj[u].append(e1);
    m_adj[v].append(e2);
    m_degree[u]++;
    m_degree[v]++;
    m_edgeCounter++;
}

/* ---- Reset graph ---- */

void EulerTour10::clear()
{
    m_adj.clear();
    m_degree.clear();
    m_numVertices = 0;
    m_edgeCounter = 0;
}

/* ---- Count unused edges from vertex ---- */

int EulerTour10::unusedEdgeCount(int v) const
{
    int count = 0;
    if (v < 0 || v >= m_adj.size()) return 0;
    for (const auto& e : m_adj[v])
        if (!e.used) count++;
    return count;
}

/* ---- Check Euler circuit ---- */

bool EulerTour10::hasEulerCircuit() const
{
    if (m_edgeCounter == 0) return false;
    // All vertices must have even degree
    for (int i = 0; i < m_numVertices; ++i)
        if (m_degree[i] % 2 != 0) return false;
    return true;
}

/* ---- Check Euler path ---- */

bool EulerTour10::hasEulerPath() const
{
    if (m_edgeCounter == 0) return false;
    int oddCount = 0;
    for (int i = 0; i < m_numVertices; ++i)
        if (m_degree[i] % 2 != 0) oddCount++;
    return (oddCount == 0 || oddCount == 2);
}

/* ---- Hierholzer's algorithm core ---- */

void EulerTour10::hierholzer(int start, QVector<int>& circuit, QVector<int>& edgePath)
{
    // Stack-based Hierholzer's algorithm
    QStack<int> stack;
    QStack<int> edgeStack;
    stack.push(start);
    edgeStack.push(-1);

    while (!stack.isEmpty()) {
        int v = stack.top();

        // Find unused edge (Fleury-like: prefer non-bridge edges)
        int edgeIdx = -1;
        int bestEdge = -1;
        bool bestIsBridge = true;

        for (int i = 0; i < m_adj[v].size(); ++i) {
            if (!m_adj[v][i].used) {
                int to = m_adj[v][i].to;
                bool isBridge = (unusedEdgeCount(v) == 1);

                // Prefer non-bridge (Fleury bridge avoidance)
                if (isBridge < bestIsBridge || bestEdge == -1) {
                    bestEdge = i;
                    bestIsBridge = isBridge;
                    if (!isBridge) break;  // Found non-bridge, use it
                }
            }
        }

        if (bestEdge >= 0) {
            Edge& e = m_adj[v][bestEdge];
            e.used = true;

            // Mark reverse edge as used
            int to = e.to;
            for (auto& re : m_adj[to]) {
                if (re.id == e.id && !re.used) {
                    re.used = true;
                    break;
                }
            }

            stack.push(to);
            edgeStack.push(e.id);
        } else {
            // No more unused edges from v, add to circuit
            if (v != start || circuit.isEmpty()) {
                circuit.append(v);
                int eid = edgeStack.top();
                if (eid >= 0) edgePath.append(eid);
            }
            stack.pop();
            edgeStack.pop();
        }
    }
}

/* ---- Find Euler tour ---- */

EulerTour10::TourResult EulerTour10::findEulerTour()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    // Reset all edge used flags
    for (auto& adjList : m_adj)
        for (auto& e : adjList)
            e.used = false;

    // Determine start vertex
    int start = 0;
    int oddVertices = 0;
    int firstOdd = -1;

    for (int i = 0; i < m_numVertices; ++i) {
        if (m_degree[i] % 2 != 0) {
            oddVertices++;
            if (firstOdd < 0) firstOdd = i;
        }
    }

    if (oddVertices == 2) {
        start = firstOdd;  // Start from odd-degree vertex
        result.isCircuit = false;
    } else if (oddVertices == 0) {
        // Find a vertex with edges
        for (int i = 0; i < m_numVertices; ++i) {
            if (m_degree[i] > 0) { start = i; break; }
        }
        result.isCircuit = true;
    } else {
        result.isValid = false;
        double elapsed = timer.elapsed();
        emit tourFound(0, 0, false, elapsed);
        return result;
    }

    QVector<int> circuit;
    QVector<int> edges;
    hierholzer(start, circuit, edges);

    // Reverse to get correct traversal order
    std::reverse(circuit.begin(), circuit.end());
    std::reverse(edges.begin(), edges.end());

    result.vertexPath = circuit;
    result.edgePath = edges;
    result.totalEdges = m_edgeCounter;
    result.isValid = (edges.size() == m_edgeCounter);

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edgeCounter;
    m_stats.totalSearches++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit tourFound(circuit.size(), edges.size(), result.isCircuit, elapsed);
    return result;
}

/* ---- Reset statistics ---- */

void EulerTour10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
