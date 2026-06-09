/**
 * @file EulerTour6.cpp
 * @brief EulerTour6 实现
 *
 * 实现欧拉回路：Hierholzer算法与Fleury边选择启发式。
 */

#include "utils/graph253/EulerTour6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EulerTour6::EulerTour6(QObject *parent) : QObject(parent) {}
EulerTour6::~EulerTour6() = default;

/* ---- Graph building ---- */

void EulerTour6::buildDirected(int n)
{
    m_n = n; m_directed = true; m_edgeCounter = 0;
    m_adj.assign(n, QVector<Edge>());
    m_edgeCount.assign(n, 0);
}

void EulerTour6::buildUndirected(int n)
{
    m_n = n; m_directed = false; m_edgeCounter = 0;
    m_adj.assign(n, QVector<Edge>());
    m_edgeCount.assign(n, 0);
}

void EulerTour6::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    int eid = m_edgeCounter++;
    m_adj[u].append({v, eid, false});
    m_edgeCount[u]++;

    if (!m_directed) {
        m_adj[v].append({u, eid, false});
        m_edgeCount[v]++;
    }
}

/* ---- Reachability count ---- */

int EulerTour6::countReachable(int v, QVector<bool>& visited) const
{
    int count = 1;
    visited[v] = true;
    for (const auto& e : m_adj[v]) {
        if (!visited[e.to] && !e.used)
            count += countReachable(e.to, visited);
    }
    return count;
}

/* ---- Bridge check ---- */

bool EulerTour6::isBridge(int u, int edgeIdx)
{
    if (m_directed) return false;  // Bridge check only for undirected

    // Temporarily remove edge and check connectivity
    m_adj[u][edgeIdx].used = true;
    int v = m_adj[u][edgeIdx].to;

    // Find matching edge in v's adjacency
    int vIdx = -1;
    for (int i = 0; i < m_adj[v].size(); ++i) {
        if (m_adj[v][i].id == m_adj[u][edgeIdx].id && !m_adj[v][i].used) {
            vIdx = i; break;
        }
    }
    if (vIdx >= 0) m_adj[v][vIdx].used = true;

    // Count reachable from u
    QVector<bool> visited(m_n, false);
    int reachable = 0;
    for (int i = 0; i < m_n; ++i) {
        for (const auto& e : m_adj[i]) {
            if (!e.used) { reachable++; break; }
        }
    }
    int countU = countReachable(u, visited);

    // Restore edges
    m_adj[u][edgeIdx].used = false;
    if (vIdx >= 0) m_adj[v][vIdx].used = false;

    // If removing edge significantly reduces reachability, it's a bridge
    return countU < (reachable / 2 + 1);
}

/* ---- Fleury edge selection ---- */

int EulerTour6::fleurySelectEdge(int u)
{
    int nonBridgeEdge = -1;
    int bridgeEdge = -1;

    for (int i = 0; i < m_adj[u].size(); ++i) {
        if (m_adj[u][i].used) continue;
        if (isBridge(u, i)) {
            bridgeEdge = i;
        } else {
            nonBridgeEdge = i;
            break;  // Prefer first non-bridge
        }
    }

    if (nonBridgeEdge >= 0) return nonBridgeEdge;
    return bridgeEdge;
}

/* ---- Hierholzer for directed ---- */

QVector<int> EulerTour6::hierholzerDirected()
{
    // Find start vertex (for circuit: any with out-degree > 0)
    int start = 0;
    int startOdd = -1, endOdd = -1;
    for (int i = 0; i < m_n; ++i) {
        int outDeg = 0, inDeg = 0;
        for (const auto& e : m_adj[i]) if (!e.used) outDeg++;
        for (int j = 0; j < m_n; ++j)
            for (const auto& e : m_adj[j])
                if (e.to == i && !e.used) inDeg++;

        if (outDeg - inDeg == 1) startOdd = i;
        if (inDeg - outDeg == 1) endOdd = i;
    }

    start = (startOdd >= 0) ? startOdd : 0;

    // Hierholzer: build tour using stack
    QVector<int> tour;
    QVector<int> stack;
    stack.append(start);

    QVector<int> edgePtr(m_n, 0);

    while (!stack.isEmpty()) {
        int v = stack.last();
        int& ptr = edgePtr[v];

        // Find next unused edge
        while (ptr < m_adj[v].size() && m_adj[v][ptr].used) ptr++;

        if (ptr < m_adj[v].size()) {
            int w = m_adj[v][ptr].to;
            m_adj[v][ptr].used = true;
            ptr++;
            stack.append(w);
        } else {
            tour.append(stack.last());
            stack.removeLast();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Hierholzer with Fleury for undirected ---- */

QVector<int> EulerTour6::hierholzerUndirected()
{
    // Find start vertex for Euler path (odd degree vertex)
    int start = 0;
    int oddCount = 0;
    for (int i = 0; i < m_n; ++i) {
        int deg = 0;
        for (const auto& e : m_adj[i]) if (!e.used) deg++;
        if (deg % 2 == 1) { start = i; oddCount++; }
    }

    QVector<int> tour;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.last();

        // Use Fleury's heuristic to select edge
        int edgeIdx = fleurySelectEdge(v);

        if (edgeIdx >= 0) {
            int w = m_adj[v][edgeIdx].to;
            int eid = m_adj[v][edgeIdx].id;

            // Mark both directions
            m_adj[v][edgeIdx].used = true;
            for (int i = 0; i < m_adj[w].size(); ++i) {
                if (m_adj[w][i].id == eid && !m_adj[w][i].used) {
                    m_adj[w][i].used = true;
                    break;
                }
            }

            m_stats.numBridgesAvoided++;
            emit edgeTraversed(v, w, eid);
            stack.append(w);
        } else {
            tour.append(stack.last());
            stack.removeLast();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Check conditions ---- */

bool EulerTour6::hasEulerCircuit() const
{
    if (m_directed) {
        for (int i = 0; i < m_n; ++i) {
            int outDeg = 0, inDeg = 0;
            for (const auto& e : m_adj[i]) outDeg++;
            for (int j = 0; j < m_n; ++j)
                for (const auto& e : m_adj[j])
                    if (e.to == i) inDeg++;
            if (outDeg != inDeg) return false;
        }
        return true;
    } else {
        for (int i = 0; i < m_n; ++i)
            if (m_adj[i].size() % 2 != 0) return false;
        return true;
    }
}

bool EulerTour6::hasEulerPath() const
{
    if (hasEulerCircuit()) return true;
    if (m_directed) {
        int startOdds = 0, endOdds = 0;
        for (int i = 0; i < m_n; ++i) {
            int outDeg = m_adj[i].size();
            int inDeg = 0;
            for (int j = 0; j < m_n; ++j)
                for (const auto& e : m_adj[j])
                    if (e.to == i) inDeg++;
            if (outDeg - inDeg == 1) startOdds++;
            if (inDeg - outDeg == 1) endOdds++;
        }
        return startOdds == 1 && endOdds == 1;
    } else {
        int oddCount = 0;
        for (int i = 0; i < m_n; ++i)
            if (m_adj[i].size() % 2 != 0) oddCount++;
        return oddCount == 2;
    }
}

/* ---- Find tour ---- */

QVector<int> EulerTour6::findTour()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> tour;
    if (m_directed)
        tour = hierholzerDirected();
    else
        tour = hierholzerUndirected();

    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCounter;
    m_stats.tourLength = tour.size();
    m_stats.hasEulerTour = !tour.isEmpty();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourFound(tour.size(), hasEulerCircuit(), timer.elapsed());
    return tour;
}

/* ---- Reset ---- */

void EulerTour6::resetStatistics()
{
    m_adj.clear(); m_edgeCount.clear();
    m_n = 0; m_edgeCounter = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
