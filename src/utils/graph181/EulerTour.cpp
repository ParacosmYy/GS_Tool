/**
 * @file EulerTour.cpp
 * @brief EulerTour 实现
 *
 * 实现欧拉回路/路径：Hierholzer算法、Fleury回退、割边检测。
 */

#include "utils/graph181/EulerTour.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

EulerTour::EulerTour(QObject* parent)
    : QObject(parent)
{
}

EulerTour::~EulerTour() = default;

void EulerTour::setGraphType(GraphType type)
{
    m_graphType = type;
}

void EulerTour::addEdge(int from, int to)
{
    m_rawEdges.append(qMakePair(from, to));
}

void EulerTour::clear()
{
    m_adj.clear();
    m_rawEdges.clear();
}

EulerTour::EulerType EulerTour::checkEuler(int vertexCount) const
{
    /* Build adjacency for degree check */
    QVector<int> inDeg(vertexCount, 0);
    QVector<int> outDeg(vertexCount, 0);

    for (const auto& e : m_rawEdges) {
        if (e.first < 0 || e.first >= vertexCount ||
            e.second < 0 || e.second >= vertexCount)
            return None;
        outDeg[e.first]++;
        inDeg[e.second]++;
    }

    if (m_graphType == Undirected) {
        int oddCount = 0;
        for (int i = 0; i < vertexCount; ++i) {
            int deg = inDeg[i] + outDeg[i]; /* Treat as degree sum */
            if (deg % 2 != 0) oddCount++;
        }
        /* In undirected: inDeg==outDeg for edge (u,v) increments both */
        /* Recalculate: each edge (u,v) contributes 1 to degree of u and v */
        QVector<int> deg(vertexCount, 0);
        for (const auto& e : m_rawEdges) {
            deg[e.first]++;
            deg[e.second]++;
        }
        oddCount = 0;
        for (int i = 0; i < vertexCount; ++i)
            if (deg[i] % 2 != 0) oddCount++;

        if (oddCount == 0) return Circuit;
        if (oddCount == 2) return Path;
        return None;
    } else {
        /* Directed: Euler circuit iff inDeg==outDeg for all */
        int startEnd = 0, endStart = 0;
        for (int i = 0; i < vertexCount; ++i) {
            if (inDeg[i] == outDeg[i]) continue;
            if (outDeg[i] == inDeg[i] + 1) startEnd++;
            else if (inDeg[i] == outDeg[i] + 1) endStart++;
            else return None;
        }
        if (startEnd == 0 && endStart == 0) return Circuit;
        if (startEnd == 1 && endStart == 1) return Path;
        return None;
    }
}

int EulerTour::findStartVertex(int vertexCount) const
{
    QVector<int> inDeg(vertexCount, 0);
    QVector<int> outDeg(vertexCount, 0);
    for (const auto& e : m_rawEdges) {
        outDeg[e.first]++;
        inDeg[e.second]++;
    }

    if (m_graphType == Undirected) {
        for (const auto& e : m_rawEdges) {
            QVector<int> deg(vertexCount, 0);
            for (const auto& ed : m_rawEdges) { deg[ed.first]++; deg[ed.second]++; }
            for (int i = 0; i < vertexCount; ++i)
                if (deg[i] % 2 != 0) return i;
            return e.first;
        }
    } else {
        for (int i = 0; i < vertexCount; ++i)
            if (outDeg[i] > inDeg[i]) return i;
    }
    return 0;
}

QVector<int> EulerTour::hierholzer(int vertexCount, int startVertex)
{
    /* Build adjacency list with edge markers */
    m_adj.clear();
    m_adj.resize(vertexCount);

    int edgeIdx = 0;
    for (const auto& e : m_rawEdges) {
        Edge fwd;
        fwd.to = e.second;
        fwd.rev = m_adj[e.second].size();
        fwd.used = false;

        Edge bwd;
        bwd.to = e.first;
        bwd.rev = m_adj[e.first].size();
        bwd.used = false;

        m_adj[e.first].append(fwd);
        if (m_graphType == Undirected)
            m_adj[e.second].append(bwd);
        edgeIdx++;
    }

    /* Hierholzer: stack-based */
    QVector<int> path;
    QVector<int> stack;
    stack.append(startVertex);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        for (auto& e : m_adj[v]) {
            if (!e.used) {
                e.used = true;
                if (m_graphType == Undirected) {
                    /* Mark reverse edge as used */
                    if (e.to < vertexCount && e.rev < m_adj[e.to].size())
                        m_adj[e.to][e.rev].used = true;
                }
                stack.append(e.to);
                found = true;
                break;
            }
        }
        if (!found) {
            path.append(v);
            stack.removeLast();
        }
    }

    /* Reverse to get correct order */
    std::reverse(path.begin(), path.end());
    return path;
}

QVector<int> EulerTour::findEulerTour(int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    EulerType type = checkEuler(vertexCount);
    if (type == None) return QVector<int>();

    int start = findStartVertex(vertexCount);
    QVector<int> tour = hierholzer(vertexCount, start);

    m_stats.totalRuns++;
    m_stats.lastVertexCount = vertexCount;
    m_stats.lastEdgeCount = m_rawEdges.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit tourFound(m_rawEdges.size(), static_cast<int>(type));
    return tour;
}

int EulerTour::dfsCount(int v, QVector<bool>& visited,
                        const QVector<QVector<int>>& adj) const
{
    visited[v] = true;
    int count = 1;
    for (int nb : adj[v]) {
        if (!visited[nb]) count += dfsCount(nb, visited, adj);
    }
    return count;
}

bool EulerTour::isBridge(int vertexCount, int u, int v) const
{
    QVector<QVector<int>> adj(vertexCount);
    for (const auto& e : m_rawEdges) {
        if ((e.first == u && e.second == v) ||
            (m_graphType == Undirected && e.first == v && e.second == u))
            continue;
        adj[e.first].append(e.second);
        if (m_graphType == Undirected)
            adj[e.second].append(e.first);
    }

    QVector<bool> visited(vertexCount, false);
    int cnt1 = dfsCount(u, visited, adj);
    visited.fill(false);
    /* Rebuild with edge */
    QVector<QVector<int>> adjFull(vertexCount);
    for (const auto& e : m_rawEdges) {
        adjFull[e.first].append(e.second);
        if (m_graphType == Undirected)
            adjFull[e.second].append(e.first);
    }
    int cnt2 = dfsCount(u, visited, adjFull);
    return cnt1 < cnt2;
}

QVector<int> EulerTour::findEulerTourFleury(int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    EulerType type = checkEuler(vertexCount);
    if (type == None) return QVector<int>();

    /* Build adjacency list */
    QVector<QVector<int>> adj(vertexCount);
    QVector<QPair<int, int>> edges = m_rawEdges;

    for (const auto& e : edges) {
        adj[e.first].append(e.second);
        if (m_graphType == Undirected)
            adj[e.second].append(e.first);
    }

    int current = findStartVertex(vertexCount);
    QVector<int> path;
    path.append(current);

    int remaining = edges.size();
    while (remaining > 0) {
        int next = -1;
        int edgeIdx = -1;
        /* Prefer non-bridge edges */
        for (int i = 0; i < adj[current].size(); ++i) {
            int nb = adj[current][i];
            if (nb < 0) continue;
            if (remaining == 1 || !isBridge(vertexCount, current, nb)) {
                next = nb;
                adj[current][i] = -1;
                /* Remove reverse */
                for (int j = 0; j < adj[nb].size(); ++j) {
                    if (adj[nb][j] == current) { adj[nb][j] = -1; break; }
                }
                remaining--;
                break;
            }
        }
        if (next < 0 && remaining > 0) {
            /* Fallback: take any remaining edge */
            for (int i = 0; i < adj[current].size(); ++i) {
                if (adj[current][i] >= 0) {
                    next = adj[current][i];
                    adj[current][i] = -1;
                    for (int j = 0; j < adj[next].size(); ++j) {
                        if (adj[next][j] == current) { adj[next][j] = -1; break; }
                    }
                    remaining--;
                    break;
                }
            }
        }
        if (next < 0) break;
        current = next;
        path.append(current);
    }

    m_stats.totalRuns++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit tourFound(m_rawEdges.size(), static_cast<int>(type));
    return path;
}

void EulerTour::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
