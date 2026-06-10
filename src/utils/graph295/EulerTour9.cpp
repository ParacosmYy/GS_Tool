/**
 * @file EulerTour9.cpp
 * @brief EulerTour9 实现
 *
 * 实现欧拉游走：Hierholzer顶点栈回路查找与邻接表边删除的高效欧拉路径/回路。
 */

#include "utils/graph295/EulerTour9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EulerTour9::EulerTour9(QObject *parent)
    : QObject(parent) {}

EulerTour9::~EulerTour9() = default;

/* ---- Configuration ---- */

void EulerTour9::setNumVertices(int n)
{
    m_n = qMax(1, n);
    m_adj = QVector<QVector<Edge>>(m_n);
    m_degree = QVector<int>(m_n, 0);
    m_edgeCounter = 0;
}

/* ---- Add undirected edge ---- */

void EulerTour9::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    int eid = m_edgeCounter++;
    // Add edge u -> v
    Edge e1;
    e1.to = v;
    e1.edgeId = eid;
    e1.used = false;
    m_adj[u].append(e1);

    // Add edge v -> u
    Edge e2;
    e2.to = u;
    e2.edgeId = eid;
    e2.used = false;
    m_adj[v].append(e2);

    m_degree[u]++;
    m_degree[v]++;
}

/* ---- Build graph from edge list ---- */

void EulerTour9::buildGraph(const QVector<QPair<int, int>>& edges)
{
    m_adj = QVector<QVector<Edge>>(m_n);
    m_degree = QVector<int>(m_n, 0);
    m_edgeCounter = 0;

    for (const auto& e : edges)
        addEdge(e.first, e.second);
}

/* ---- Delete used edge from adjacency list ---- */

void EulerTour9::deleteEdge(int u, int edgeId)
{
    for (int i = 0; i < m_adj[u].size(); ++i) {
        if (m_adj[u][i].edgeId == edgeId) {
            m_adj[u][i].used = true;
            break;
        }
    }
}

/* ---- Check for Eulerian circuit: all vertices even degree ---- */

bool EulerTour9::hasEulerianCircuit() const
{
    if (m_n == 0) return false;
    // All non-isolated vertices must have even degree
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] % 2 != 0) return false;
    }
    return true;
}

/* ---- Check for Eulerian path: exactly 0 or 2 vertices odd degree ---- */

bool EulerTour9::hasEulerianPath() const
{
    if (m_n == 0) return false;
    int oddCount = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_degree[i] % 2 != 0) oddCount++;
    }
    return (oddCount == 0 || oddCount == 2);
}

/* ---- Find starting vertex for Eulerian path (odd degree vertex) ---- */

int EulerTour9::findPathStart() const
{
    // Start from first odd-degree vertex; fall back to first non-zero degree
    for (int i = 0; i < m_n; ++i)
        if (m_degree[i] % 2 != 0) return i;
    for (int i = 0; i < m_n; ++i)
        if (m_degree[i] > 0) return i;
    return 0;
}

/* ---- Hierholzer algorithm with vertex stack ---- */

QVector<int> EulerTour9::hierholzer(int startVertex)
{
    // Clone adjacency lists for local modification
    auto adj = m_adj;
    QVector<int> tour;           // Result path
    QVector<int> stack;          // Vertex stack
    stack.append(startVertex);

    while (!stack.isEmpty()) {
        int v = stack.back();

        // Find next unused edge from v
        int nextEid = -1;
        int nextTo = -1;
        int edgeIdx = -1;
        for (int i = 0; i < adj[v].size(); ++i) {
            if (!adj[v][i].used) {
                nextEid = adj[v][i].edgeId;
                nextTo = adj[v][i].to;
                edgeIdx = i;
                break;
            }
        }

        if (nextTo < 0) {
            // No more edges from v: add to tour
            tour.append(v);
            stack.removeLast();
        } else {
            // Mark edge used in both directions
            adj[v][edgeIdx].used = true;
            // Mark reverse edge
            for (int i = 0; i < adj[nextTo].size(); ++i) {
                if (adj[nextTo][i].edgeId == nextEid && !adj[nextTo][i].used) {
                    adj[nextTo][i].used = true;
                    break;
                }
            }
            stack.append(nextTo);
            emit vertexVisited(nextTo, stack.size());
        }
    }

    return tour;
}

/* ---- Find Eulerian circuit ---- */

EulerTour9::TourResult EulerTour9::findEulerianCircuit()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    if (!hasEulerianCircuit() && !hasEulerianPath()) {
        result.isEulerian = false;
        return result;
    }

    // Start from any vertex with non-zero degree
    int start = 0;
    for (int i = 0; i < m_n; ++i)
        if (m_degree[i] > 0) { start = i; break; }

    result.vertexPath = hierholzer(start);
    result.isEulerian = true;
    result.isCircuit = (!result.vertexPath.isEmpty() &&
                        result.vertexPath.first() == result.vertexPath.last());

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCounter;
    m_stats.tourLength = result.vertexPath.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourFound(result.vertexPath.size(), m_edgeCounter, result.isCircuit, elapsed);

    return result;
}

/* ---- Find Eulerian path ---- */

EulerTour9::TourResult EulerTour9::findEulerianPath()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    if (!hasEulerianPath()) {
        result.isEulerian = false;
        return result;
    }

    int start = findPathStart();
    result.vertexPath = hierholzer(start);
    result.isEulerian = true;
    result.isCircuit = (!result.vertexPath.isEmpty() &&
                        result.vertexPath.first() == result.vertexPath.last());

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edgeCounter;
    m_stats.tourLength = result.vertexPath.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourFound(result.vertexPath.size(), m_edgeCounter, result.isCircuit, elapsed);

    return result;
}

/* ---- Reset ---- */

void EulerTour9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_degree.clear();
    m_edgeCounter = 0;
}
