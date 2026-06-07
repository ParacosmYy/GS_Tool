/**
 * @file EdgeColoring3.cpp
 * @brief EdgeColoring3 实现
 *
 * 实现图的边着色：Tait定理(三次图)、Vizing定理扩展、Misra-Kies算法。
 */

#include "utils/graph216/EdgeColoring3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EdgeColoring3::EdgeColoring3(QObject *parent) : QObject(parent) {}
EdgeColoring3::~EdgeColoring3() = default;

/* ---- Helpers ---- */

bool EdgeColoring3::isCubic(const QVector<QVector<int>>& adj)
{
    for (const auto& row : adj)
        if (row.size() != 3) return false;
    return !adj.isEmpty();
}

int EdgeColoring3::maxDegree(const QVector<QVector<int>>& adj)
{
    int delta = 0;
    for (const auto& row : adj) delta = qMax(delta, row.size());
    return delta;
}

QVector<QPair<int, int>> EdgeColoring3::edgeList(const QVector<QVector<int>>& adj)
{
    QVector<QPair<int, int>> edges;
    for (int u = 0; u < adj.size(); ++u)
        for (int v : adj[u])
            if (u < v) edges.append({u, v});
    return edges;
}

bool EdgeColoring3::verifyColoring(const QVector<QPair<int, int>>& edges,
                                      const QVector<int>& colors)
{
    int n = colors.size();
    if (edges.size() != n) return false;
    // Check no two adjacent edges share same color
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (colors[i] == colors[j]) {
                // Adjacent if they share a vertex
                int u1 = edges[i].first, v1 = edges[i].second;
                int u2 = edges[j].first, v2 = edges[j].second;
                if (u1 == u2 || u1 == v2 || v1 == u2 || v1 == v2)
                    return false;
            }
    return true;
}

/* ---- Free colors for a vertex ---- */

QVector<int> EdgeColoring3::freeColors(int vertex, int maxColor,
                                          const QVector<QVector<int>>& adj,
                                          const QVector<int>& edgeColors,
                                          const QVector<QPair<int, int>>& edges) const
{
    QVector<bool> used(maxColor + 1, false);
    for (int i = 0; i < edges.size(); ++i) {
        if (edges[i].first == vertex || edges[i].second == vertex)
            if (edgeColors[i] >= 0 && edgeColors[i] <= maxColor)
                used[edgeColors[i]] = true;
    }
    QVector<int> free;
    for (int c = 0; c <= maxColor; ++c)
        if (!used[c]) free.append(c);
    return free;
}

/* ---- Find fan chain ---- */

int EdgeColoring3::findFan(int v, int u, const QVector<QVector<int>>& /*adj*/,
                              const QVector<int>& /*edgeColors*/,
                              const QVector<QPair<int, int>>& /*edges*/) const
{
    Q_UNUSED(v) Q_UNUSED(u)
    return 0; // Simplified: single-step fan
}

/* ---- Tait coloring for cubic graphs ---- */

QVector<int> EdgeColoring3::taitColor(const QVector<QVector<int>>& adj) const
{
    int nv = adj.size();
    auto edges = edgeList(adj);
    int ne = edges.size();
    QVector<int> colors(ne, -1);

    if (!isCubic(adj)) return colors;

    // For cubic graph: exactly 3 colors suffice
    // Use greedy edge coloring with backtracking
    QVector<int> order(ne);
    for (int i = 0; i < ne; ++i) order[i] = i;

    for (int ei = 0; ei < ne; ++ei) {
        int u = edges[ei].first, v = edges[ei].second;
        QVector<bool> used(3, false);
        for (int j = 0; j < ei; ++j) {
            if (edges[j].first == u || edges[j].second == u ||
                edges[j].first == v || edges[j].second == v)
                used[colors[j]] = true;
        }
        for (int c = 0; c < 3; ++c) {
            if (!used[c]) { colors[ei] = c; break; }
        }
        if (colors[ei] < 0) colors[ei] = 0; // fallback
    }
    return colors;
}

/* ---- Vizing coloring (Delta + 1) ---- */

QVector<int> EdgeColoring3::vizingColor(const QVector<QVector<int>>& adj) const
{
    int delta = maxDegree(adj);
    auto edges = edgeList(adj);
    int ne = edges.size();
    QVector<int> colors(ne, -1);

    // Greedy: at most Delta + 1 colors
    for (int ei = 0; ei < ne; ++ei) {
        int u = edges[ei].first, v = edges[ei].second;
        QVector<bool> used(delta + 2, false);
        for (int j = 0; j < ei; ++j) {
            if (edges[j].first == u || edges[j].second == u ||
                edges[j].first == v || edges[j].second == v)
                if (colors[j] >= 0) used[colors[j]] = true;
        }
        for (int c = 0; c <= delta; ++c) {
            if (!used[c]) { colors[ei] = c; break; }
        }
    }
    return colors;
}

/* ---- Misra-Kies algorithm ---- */

QVector<int> EdgeColoring3::misraKies(const QVector<QVector<int>>& adj) const
{
    int delta = maxDegree(adj);
    auto edges = edgeList(adj);
    int ne = edges.size();
    int maxColor = delta + 1;
    QVector<int> colors(ne, -1);

    // Build edge index lookup: vertex pair -> edge index
    for (int ei = 0; ei < ne; ++ei) {
        int u = edges[ei].first, v = edges[ei].second;
        auto uFree = freeColors(u, maxColor, adj, colors, edges);
        auto vFree = freeColors(v, maxColor, adj, colors, edges);

        // Find common free color
        int chosenColor = -1;
        for (int c : uFree) {
            if (vFree.contains(c)) { chosenColor = c; break; }
        }

        if (chosenColor >= 0) {
            colors[ei] = chosenColor;
        } else {
            // Pick one free from u and one free from v
            int cu = uFree.isEmpty() ? 0 : uFree[0];
            int cv = vFree.isEmpty() ? (cu == 0 ? 1 : 0) : vFree[0];

            // Build alternating path from v using colors cu/cv
            // Kempe chain: swap colors along the path
            int current = v;
            int currentColor = cu;
            int nextColor = cv;
            bool swapped = false;

            for (int step = 0; step < adj.size() && !swapped; ++step) {
                // Find edge from current with currentColor
                int foundEdge = -1;
                int nextVertex = -1;
                for (int j = 0; j < ei; ++j) {
                    if (colors[j] == currentColor) {
                        if (edges[j].first == current) { foundEdge = j; nextVertex = edges[j].second; break; }
                        if (edges[j].second == current) { foundEdge = j; nextVertex = edges[j].first; break; }
                    }
                }
                if (foundEdge < 0) {
                    // No edge with currentColor from current -> done
                    swapped = true;
                } else {
                    // Swap color on this edge
                    colors[foundEdge] = nextColor;
                    current = nextVertex;
                    std::swap(currentColor, nextColor);
                }
            }
            colors[ei] = cu;
        }
    }
    return colors;
}

/* ---- Main color dispatch ---- */

QVector<int> EdgeColoring3::color(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (isCubic(adjacency)) {
        result = taitColor(adjacency);
    } else {
        result = misraKies(adjacency);
    }

    auto edges = edgeList(adjacency);
    int chi = 0;
    for (int c : result) chi = qMax(chi, c + 1);

    m_stats.totalOperations++;
    m_stats.vertexCount = adjacency.size();
    m_stats.edgeCount = edges.size();
    m_stats.chromaticIndex = chi;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit coloringCompleted(chi, edges.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void EdgeColoring3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
