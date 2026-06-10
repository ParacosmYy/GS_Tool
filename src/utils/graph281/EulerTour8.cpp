/**
 * @file EulerTour8.cpp
 * @brief EulerTour8 实现
 *
 * 实现欧拉回路：Hierholzer线性时间回路检测与Fleury桥边回避保证欧拉路径。
 */

#include "utils/graph281/EulerTour8.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EulerTour8::EulerTour8(QObject *parent)
    : QObject(parent) {}

EulerTour8::~EulerTour8() = default;

/* ---- Graph initialization ---- */

void EulerTour8::initGraph(int numVertices)
{
    m_n = qMax(1, numVertices);
    m_edgeCount = 0;
    m_adj.clear();
    m_adj.resize(m_n);
    m_degree.resize(m_n, 0);
    m_edgeUsed.clear();
}

/* ---- Add undirected edge ---- */

void EulerTour8::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    int eid = m_edgeCount;
    m_edgeUsed.append(false);

    Edge e1{v, eid, false};
    Edge e2{u, eid, false};
    m_adj[u].append(e1);
    m_adj[v].append(e2);

    m_degree[u]++;
    m_degree[v]++;
    m_edgeCount++;
}

/* ---- Hierholzer DFS ---- */

void EulerTour8::hierholzerDFS(int v, QVector<int>& path)
{
    // Use iterative approach with explicit stack to avoid deep recursion
    QVector<int> stack;
    stack.append(v);

    while (!stack.isEmpty()) {
        int curr = stack.last();
        bool found = false;

        while (!m_adj[curr].isEmpty()) {
            Edge& e = m_adj[curr].last();
            if (m_edgeUsed[e.id]) {
                m_adj[curr].removeLast();
                continue;
            }
            // Mark edge as used
            m_edgeUsed[e.id] = true;
            int next = e.to;
            m_adj[curr].removeLast();

            // Also remove reverse edge from neighbor
            for (int i = m_adj[next].size() - 1; i >= 0; --i) {
                if (m_adj[next][i].id == e.id) {
                    m_adj[next].removeAt(i);
                    break;
                }
            }

            stack.append(next);
            found = true;
            break;
        }

        if (!found) {
            path.append(curr);
            stack.removeLast();
        }
    }
}

/* ---- Check Euler circuit existence ---- */

bool EulerTour8::hasEulerCircuit() const
{
    if (m_n == 0) return false;
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] % 2 != 0) return false;
    }
    return true;
}

/* ---- Check Euler path existence ---- */

bool EulerTour8::hasEulerPath() const
{
    if (m_n == 0) return false;
    int oddCount = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] % 2 != 0) oddCount++;
    }
    return (oddCount == 0 || oddCount == 2);
}

/* ---- Count reachable vertices ---- */

int EulerTour8::countReachable(int start, int excludeU, int excludeV) const
{
    QVector<bool> visited(m_n, false);
    QVector<int> stack;
    stack.append(start);
    visited[start] = true;
    int count = 0;

    while (!stack.isEmpty()) {
        int curr = stack.takeLast();
        count++;
        for (const Edge& e : m_adj[curr]) {
            if (visited[e.to]) continue;
            // Skip excluded edge
            if ((curr == excludeU && e.to == excludeV) ||
                (curr == excludeV && e.to == excludeU)) continue;
            visited[e.to] = true;
            stack.append(e.to);
        }
    }
    return count;
}

/* ---- Check if edge is a bridge (Fleury's criterion) ---- */

bool EulerTour8::isBridge(int u, int v) const
{
    if (m_adj[u].size() == 1) return true; // Only edge, must be bridge

    int before = countReachable(u, -1, -1);
    int after = countReachable(u, u, v);
    return (after < before);
}

/* ---- Find Euler circuit (Hierholzer) ---- */

QVector<int> EulerTour8::findEulerCircuit()
{
    QElapsedTimer timer;
    timer.start();

    // Reset edge usage
    m_edgeUsed.fill(false, m_edgeCount);

    // Rebuild adjacency (remove used markers)
    // Find a vertex with non-zero degree to start
    int start = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] > 0) { start = i; break; }
    }

    QVector<int> path;
    hierholzerDFS(start, path);

    // Path is in reverse order
    std::reverse(path.begin(), path.end());

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCount;
    m_stats.pathLength = path.size();
    m_stats.hasEulerCircuit = hasEulerCircuit();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourComputed(path.size(), m_stats.hasEulerCircuit, elapsed);

    return path;
}

/* ---- Find Euler path (Hierholzer with odd-degree start) ---- */

QVector<int> EulerTour8::findEulerPath()
{
    QElapsedTimer timer;
    timer.start();

    m_edgeUsed.fill(false, m_edgeCount);

    // Find start vertex: odd degree vertex if exists
    int start = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] % 2 != 0) { start = i; break; }
    }

    QVector<int> path;
    hierholzerDFS(start, path);
    std::reverse(path.begin(), path.end());

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCount;
    m_stats.pathLength = path.size();
    m_stats.hasEulerCircuit = (path.size() > 0 && path.first() == path.last());
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourComputed(path.size(), m_stats.hasEulerCircuit, elapsed);

    return path;
}

/* ---- Reset ---- */

void EulerTour8::resetStatistics()
{
    m_n = 0;
    m_edgeCount = 0;
    m_adj.clear();
    m_degree.clear();
    m_edgeUsed.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
