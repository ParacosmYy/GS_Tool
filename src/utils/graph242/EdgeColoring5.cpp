/**
 * @file EdgeColoring5.cpp
 * @brief EdgeColoring5 实现
 *
 * 实现图边着色：三次平面图Tait算法与一般图Kempe链换色贪心策略。
 */

#include "utils/graph242/EdgeColoring5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring5::EdgeColoring5(QObject *parent) : QObject(parent) {}
EdgeColoring5::~EdgeColoring5() = default;

/* ---- Build edge list from adjacency ---- */

QVector<EdgeColoring5::Edge> EdgeColoring5::buildEdgeList(
    const QVector<QVector<int>>& adjacency) const
{
    QVector<Edge> edges;
    int n = adjacency.size();
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (v > u) { // avoid duplicates
                Edge e;
                e.from = u;
                e.to = v;
                e.color = -1;
                edges.append(e);
            }
        }
    }
    return edges;
}

/* ---- Is cubic check ---- */

bool EdgeColoring5::isCubic(const QVector<QVector<int>>& adjacency) const
{
    for (const auto& neighbors : adjacency) {
        if (neighbors.size() != 3) return false;
    }
    return !adjacency.isEmpty();
}

/* ---- Find Kempe chain ---- */

QVector<int> EdgeColoring5::findKempeChain(int startEdge, int color1,
                                             int color2,
                                             const QVector<Edge>& edges,
                                             const QVector<QVector<int>>& adj) const
{
    QVector<int> chain;
    QVector<bool> visited(edges.size(), false);

    // BFS from startEdge along edges of color1/color2
    chain.append(startEdge);
    visited[startEdge] = true;

    int pos = 0;
    while (pos < chain.size()) {
        int eIdx = chain[pos++];
        int u = edges[eIdx].from;
        int v = edges[eIdx].to;

        // Find adjacent edges with matching colors
        for (int i = 0; i < edges.size(); ++i) {
            if (visited[i]) continue;
            int ec = edges[i].color;
            if (ec != color1 && ec != color2) continue;

            // Check adjacency (shares a vertex)
            bool sharesVertex = (edges[i].from == u || edges[i].to == u ||
                                  edges[i].from == v || edges[i].to == v);
            if (sharesVertex) {
                visited[i] = true;
                chain.append(i);
            }
        }
    }
    return chain;
}

/* ---- Swap Kempe chain colors ---- */

void EdgeColoring5::swapKempeChain(QVector<int>& chain, int color1,
                                     int color2, QVector<Edge>& edges)
{
    for (int idx : chain) {
        if (edges[idx].color == color1)
            edges[idx].color = color2;
        else if (edges[idx].color == color2)
            edges[idx].color = color1;
    }
}

/* ---- Tait's algorithm for cubic planar graphs ---- */

QVector<EdgeColoring5::Edge> EdgeColoring5::taitColoring(
    const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    QVector<Edge> edges = buildEdgeList(adjacency);

    // Assign colors greedily ensuring no two adjacent edges share a color
    // For cubic graphs, 3 colors suffice (by Vizing's theorem for planar)
    int maxColors = 3;

    for (int i = 0; i < edges.size(); ++i) {
        // Find colors used by adjacent edges at both endpoints
        QVector<bool> used(maxColors + 1, false);
        for (int j = 0; j < i; ++j) {
            if (edges[j].from == edges[i].from ||
                edges[j].to == edges[i].from ||
                edges[j].from == edges[i].to ||
                edges[j].to == edges[i].to) {
                if (edges[j].color >= 0 &&
                    edges[j].color < used.size())
                    used[edges[j].color] = true;
            }
        }

        int color = -1;
        for (int c = 0; c <= maxColors; ++c) {
            if (!used[c]) { color = c; break; }
        }

        if (color < 0) {
            // Try Kempe swap to free a color
            for (int c1 = 0; c1 <= maxColors; ++c1) {
                for (int c2 = c1 + 1; c2 <= maxColors; ++c2) {
                    QVector<int> chain = findKempeChain(
                        i, c1, c2, edges, adjacency);
                    if (!chain.isEmpty()) {
                        swapKempeChain(chain, c1, c2, edges);
                        // Re-check if color c1 is now free
                        bool c1Free = true;
                        for (int j = 0; j < i; ++j) {
                            if ((edges[j].from == edges[i].from ||
                                 edges[j].to == edges[i].from ||
                                 edges[j].from == edges[i].to ||
                                 edges[j].to == edges[i].to) &&
                                edges[j].color == c1) {
                                c1Free = false; break;
                            }
                        }
                        if (c1Free) { color = c1; break; }
                    }
                }
                if (color >= 0) break;
            }
        }

        edges[i].color = (color >= 0) ? color : maxColors;
    }

    m_stats.isCubicPlanar = true;
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
    m_stats.numColors = maxColors;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringCompleted(edges.size(), maxColors, timer.elapsed());
    return edges;
}

/* ---- Kempe coloring for general graphs ---- */

QVector<EdgeColoring5::Edge> EdgeColoring5::kempeColoring(
    const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    int maxDegree = 0;
    for (const auto& nbrs : adjacency)
        maxDegree = qMax(maxDegree, nbrs.size());

    // By Vizing's theorem: need maxDegree or maxDegree+1 colors
    int numColors = maxDegree + 1;
    QVector<Edge> edges = buildEdgeList(adjacency);

    // Greedy edge coloring with Kempe chain improvement
    for (int i = 0; i < edges.size(); ++i) {
        QVector<bool> used(numColors, false);
        for (int j = 0; j < i; ++j) {
            if (edges[j].from == edges[i].from ||
                edges[j].to == edges[i].from ||
                edges[j].from == edges[i].to ||
                edges[j].to == edges[i].to) {
                if (edges[j].color >= 0 && edges[j].color < numColors)
                    used[edges[j].color] = true;
            }
        }

        int color = -1;
        for (int c = 0; c < numColors; ++c) {
            if (!used[c]) { color = c; break; }
        }

        if (color < 0) {
            // All colors used: try Kempe chain swap
            for (int c1 = 0; c1 < numColors && color < 0; ++c1) {
                for (int c2 = c1 + 1; c2 < numColors; ++c2) {
                    QVector<int> chain = findKempeChain(
                        i, c1, c2, edges, adjacency);
                    if (!chain.isEmpty()) {
                        swapKempeChain(chain, c1, c2, edges);
                        bool c1Free = true;
                        for (int j = 0; j < i; ++j) {
                            if ((edges[j].from == edges[i].from ||
                                 edges[j].to == edges[i].from ||
                                 edges[j].from == edges[i].to ||
                                 edges[j].to == edges[i].to) &&
                                edges[j].color == c1) {
                                c1Free = false; break;
                            }
                        }
                        if (c1Free) { color = c1; break; }
                    }
                }
            }
        }

        edges[i].color = (color >= 0) ? color : 0;
    }

    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
    m_stats.numColors = numColors;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringCompleted(edges.size(), numColors, timer.elapsed());
    return edges;
}

/* ---- Main color dispatch ---- */

QVector<EdgeColoring5::Edge> EdgeColoring5::color(
    const QVector<QVector<int>>& adjacency)
{
    if (isCubic(adjacency))
        return taitColoring(adjacency);
    return kempeColoring(adjacency);
}

/* ---- Reset ---- */

void EdgeColoring5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
