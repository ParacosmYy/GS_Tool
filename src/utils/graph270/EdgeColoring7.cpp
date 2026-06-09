/**
 * @file EdgeColoring7.cpp
 * @brief EdgeColoring7 实现
 *
 * 实现边着色：Vizing定理分类与贪心近优边着色分配。
 */

#include "utils/graph270/EdgeColoring7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring7::EdgeColoring7(QObject *parent)
    : QObject(parent) {}
EdgeColoring7::~EdgeColoring7() = default;

/* ---- Configuration ---- */

void EdgeColoring7::setGraph(int numVertices, const QVector<QPair<int,int>>& edges)
{
    m_numVertices = qMax(1, numVertices);
    m_edges.clear();
    for (const auto& e : edges) {
        Edge edge;
        edge.u = e.first;
        edge.v = e.second;
        edge.color = -1;
        m_edges.append(edge);
    }
    computeMaxDegree();
}

/* ---- Compute maximum degree ---- */

void EdgeColoring7::computeMaxDegree()
{
    QVector<int> degree(m_numVertices, 0);
    for (const Edge& e : m_edges) {
        if (e.u >= 0 && e.u < m_numVertices) degree[e.u]++;
        if (e.v >= 0 && e.v < m_numVertices) degree[e.v]++;
    }
    m_maxDeg = 0;
    for (int d : degree)
        m_maxDeg = qMax(m_maxDeg, d);
}

/* ---- Vertex degree ---- */

int EdgeColoring7::vertexDegree(int v) const
{
    int deg = 0;
    for (const Edge& e : m_edges) {
        if (e.u == v || e.v == v) deg++;
    }
    return deg;
}

/* ---- Check if graph is regular ---- */

bool EdgeColoring7::isRegular() const
{
    if (m_numVertices <= 1) return true;
    int targetDeg = vertexDegree(0);
    for (int v = 1; v < m_numVertices; ++v) {
        if (vertexDegree(v) != targetDeg) return false;
    }
    return true;
}

/* ---- Check for odd cycle ---- */

bool EdgeColoring7::hasOddCycle() const
{
    // BFS-based bipartiteness check using edge adjacency
    // Build vertex adjacency
    QVector<QVector<int>> adj(m_numVertices);
    for (const Edge& e : m_edges) {
        if (e.u >= 0 && e.u < m_numVertices && e.v >= 0 && e.v < m_numVertices) {
            adj[e.u].append(e.v);
            adj[e.v].append(e.u);
        }
    }

    QVector<int> color(m_numVertices, -1);
    for (int start = 0; start < m_numVertices; ++start) {
        if (color[start] >= 0) continue;
        color[start] = 0;
        QVector<int> queue;
        queue.append(start);
        int head = 0;
        while (head < queue.size()) {
            int u = queue[head++];
            for (int v : adj[u]) {
                if (color[v] < 0) {
                    color[v] = 1 - color[u];
                    queue.append(v);
                } else if (color[v] == color[u]) {
                    return true; // Odd cycle found
                }
            }
        }
    }
    return false;
}

/* ---- Check if color available for edge ---- */

bool EdgeColoring7::isColorAvailable(int edgeIdx, int c) const
{
    const Edge& e = m_edges[edgeIdx];
    for (const Edge& other : m_edges) {
        if (other.color == c) {
            if (other.u == e.u || other.u == e.v ||
                other.v == e.u || other.v == e.v)
                return false;
        }
    }
    return true;
}

/* ---- Find smallest available color ---- */

int EdgeColoring7::findSmallestColor(int edgeIdx) const
{
    // Try colors from 0 up to maxDegree + 1 (Vizing bound)
    for (int c = 0; c <= m_maxDeg; ++c) {
        if (isColorAvailable(edgeIdx, c))
            return c;
    }
    return m_maxDeg + 1; // Class 2 graph edge
}

/* ---- Vizing classification ---- */

EdgeColoring7::GraphClass EdgeColoring7::classifyGraph() const
{
    if (m_edges.isEmpty()) return GraphClass::Unknown;

    // Vizing's theorem: chromatic index is Δ or Δ+1
    // Class 1 if Δ colors suffice, Class 2 if Δ+1 needed

    // Simple heuristic: bipartite graphs are always Class 1
    // Regular graphs of odd order are Class 2
    if (!hasOddCycle()) return GraphClass::Class1;

    if (isRegular() && (m_numVertices % 2 == 1))
        return GraphClass::Class2;

    // Default: attempt greedy and check
    return GraphClass::Unknown;
}

/* ---- Maximum degree accessor ---- */

int EdgeColoring7::maxDegree() const { return m_maxDeg; }

/* ---- Lower bound ---- */

int EdgeColoring7::lowerBound() const { return m_maxDeg; }

/* ---- Main greedy edge coloring ---- */

QVector<EdgeColoring7::Edge> EdgeColoring7::color()
{
    QElapsedTimer timer;
    timer.start();

    // Reset colors
    for (int i = 0; i < m_edges.size(); ++i)
        m_edges[i].color = -1;

    // Sort edges by descending sum of endpoint degrees (heuristic for better coloring)
    QVector<int> edgeOrder;
    edgeOrder.reserve(m_edges.size());
    for (int i = 0; i < m_edges.size(); ++i)
        edgeOrder.append(i);

    std::sort(edgeOrder.begin(), edgeOrder.end(), [this](int a, int b) {
        int da = vertexDegree(m_edges[a].u) + vertexDegree(m_edges[a].v);
        int db = vertexDegree(m_edges[b].u) + vertexDegree(m_edges[b].v);
        return da > db;
    });

    // Greedy assignment
    int maxColor = 0;
    for (int idx : edgeOrder) {
        int c = findSmallestColor(idx);
        m_edges[idx].color = c;
        maxColor = qMax(maxColor, c);
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.maxDegree = m_maxDeg;
    m_stats.colorsUsed = maxColor + 1;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit coloringCompleted(maxColor + 1, m_edges.size(), elapsed);
    return m_edges;
}

/* ---- Reset ---- */

void EdgeColoring7::resetStatistics()
{
    m_edges.clear();
    m_numVertices = 0;
    m_maxDeg = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
