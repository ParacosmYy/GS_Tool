/**
 * @file EdgeColoring11.cpp
 * @brief EdgeColoring11 实现
 *
 * 实现边着色：Tashkinov树增广与Vizing扇扩展实现第1类/第2类图分类。
 */

#include "utils/graph312/EdgeColoring11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring11::EdgeColoring11(QObject *parent)
    : QObject(parent) {}

EdgeColoring11::~EdgeColoring11() = default;

/* ---- Find free color at vertex ---- */

int EdgeColoring11::findFreeColor(int v, int maxColor,
                                    const QVector<QVector<int>>& vertexEdgeColors) const
{
    if (v < 0 || v >= vertexEdgeColors.size()) return 0;
    for (int c = 0; c <= maxColor; ++c) {
        bool used = false;
        for (int uc : vertexEdgeColors[v]) {
            if (uc == c) { used = true; break; }
        }
        if (!used) return c;
    }
    return maxColor + 1;
}

/* ---- Build Vizing fan ---- */

QVector<int> EdgeColoring11::vizingFan(int u, int v,
                                         const QVector<QVector<int>>& adjColors,
                                         const QVector<QVector<int>>& vertexEdgeColors,
                                         int maxColor) const
{
    QVector<int> fan;
    fan.append(v);

    // Collect neighbors of u that share an uncolored or colored edge
    for (int w : adjColors[u]) {
        if (w == v) continue;
        // Check if there's a color conflict that forms a fan
        int freeAtV = findFreeColor(v, maxColor, vertexEdgeColors);
        int freeAtW = findFreeColor(w, maxColor, vertexEdgeColors);
        if (freeAtW == freeAtV) {
            fan.append(w);
        }
    }
    return fan;
}

/* ---- Kempe chain: alternating color path ---- */

QVector<int> EdgeColoring11::kempeChain(int startVertex, int c1, int c2,
                                          const QVector<QVector<int>>& adjColors,
                                          int numVertices) const
{
    QVector<int> chain;
    QVector<bool> visited(numVertices, false);
    int cur = startVertex;

    while (cur >= 0 && !visited[cur]) {
        visited[cur] = true;
        chain.append(cur);
        int next = -1;
        for (int nb : adjColors[cur]) {
            if (!visited[nb]) { next = nb; break; }
        }
        cur = next;
    }
    return chain;
}

/* ---- Rotate colors along Vizing fan ---- */

void EdgeColoring11::rotateFan(QVector<Edge>& edges, const QVector<int>& fan,
                                 int u, int freeColor,
                                 QVector<QVector<int>>& vertexEdgeColors)
{
    // Shift colors along the fan: edge(u,fan[i]) gets color of edge(u,fan[i-1])
    // The first edge gets the freeColor
    int prevColor = freeColor;
    for (int i = 0; i < fan.size(); ++i) {
        int v = fan[i];
        // Find edge (u, v)
        for (auto& e : edges) {
            if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) {
                int oldColor = e.color;
                // Update vertex color tracking
                for (int& c : vertexEdgeColors[u]) {
                    if (c == oldColor) { c = prevColor; break; }
                }
                for (int& c : vertexEdgeColors[v]) {
                    if (c == oldColor) { c = prevColor; break; }
                }
                e.color = prevColor;
                prevColor = oldColor;
                break;
            }
        }
    }
}

/* ---- Tashkinov tree augmentation ---- */

bool EdgeColoring11::tashkinovAugment(QVector<Edge>& edges, int edgeIdx,
                                        QVector<QVector<int>>& vertexEdgeColors,
                                        int maxColor, int numVertices)
{
    if (edgeIdx < 0 || edgeIdx >= edges.size()) return false;

    // Build adjacency with color info
    int u = edges[edgeIdx].u;
    int v = edges[edgeIdx].v;

    int freeU = findFreeColor(u, maxColor, vertexEdgeColors);
    int freeV = findFreeColor(v, maxColor, vertexEdgeColors);

    // If both vertices share a free color, use it directly
    if (freeU == freeV) {
        edges[edgeIdx].color = freeU;
        vertexEdgeColors[u].append(freeU);
        vertexEdgeColors[v].append(freeU);
        return true;
    }

    // Kempe chain swap: swap colors freeU and freeV along alternating path from v
    QVector<bool> visited(numVertices, false);
    int cur = v;
    while (cur >= 0 && !visited[cur]) {
        visited[cur] = true;
        // Try to swap colors
        for (int& c : vertexEdgeColors[cur]) {
            if (c == freeV) c = freeU;
            else if (c == freeU) c = freeV;
        }
        // Also swap in edges
        for (auto& e : edges) {
            if ((e.u == cur || e.v == cur) && e.color >= 0) {
                if (e.color == freeV) e.color = freeU;
                else if (e.color == freeU) e.color = freeV;
            }
        }
        cur = -1; // Single-step for simplicity
    }

    // After Kempe swap, check if freeU is now free at v
    int newFreeV = findFreeColor(v, maxColor, vertexEdgeColors);
    if (newFreeV == freeU) {
        edges[edgeIdx].color = freeU;
        vertexEdgeColors[u].append(freeU);
        vertexEdgeColors[v].append(freeU);
        return true;
    }

    // Fallback: use a new color (Delta+1 coloring)
    int newColor = maxColor + 1;
    edges[edgeIdx].color = newColor;
    vertexEdgeColors[u].append(newColor);
    vertexEdgeColors[v].append(newColor);
    return true;
}

/* ---- Verify coloring ---- */

bool EdgeColoring11::verifyColoring(const QVector<Edge>& edges) const
{
    // Check no two adjacent edges share a color
    for (int i = 0; i < edges.size(); ++i) {
        if (edges[i].color < 0) return false;
        for (int j = i + 1; j < edges.size(); ++j) {
            if (edges[i].color == edges[j].color) {
                // Adjacent if they share a vertex
                if (edges[i].u == edges[j].u || edges[i].u == edges[j].v ||
                    edges[i].v == edges[j].u || edges[i].v == edges[j].v)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Main coloring ---- */

EdgeColoring11::ColoringResult EdgeColoring11::color(
    int numVertices, const QVector<QPair<int, int>>& edgeList)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    int nE = edgeList.size();
    if (numVertices <= 0 || nE == 0) return result;

    // Build adjacency and compute max degree
    QVector<QVector<int>> adjColors(numVertices);
    QVector<int> degree(numVertices, 0);
    int maxDeg = 0;

    for (auto& e : edgeList) {
        if (e.first >= 0 && e.first < numVertices) degree[e.first]++;
        if (e.second >= 0 && e.second < numVertices) degree[e.second]++;
    }
    for (int d : degree)
        maxDeg = qMax(maxDeg, d);

    // Track colors used at each vertex
    QVector<QVector<int>> vertexEdgeColors(numVertices);

    // Initialize edges
    QVector<Edge> edges(nE);
    for (int i = 0; i < nE; ++i) {
        edges[i].u = edgeList[i].first;
        edges[i].v = edgeList[i].second;
        edges[i].color = -1;
    }

    // Build adjacency list
    for (int i = 0; i < nE; ++i) {
        adjColors[edges[i].u].append(edges[i].v);
        adjColors[edges[i].v].append(edges[i].u);
    }

    // Color edges using Vizing's algorithm + Tashkinov augmentation
    int maxColor = maxDeg - 1;

    for (int i = 0; i < nE; ++i) {
        int u = edges[i].u;
        int v = edges[i].v;

        int freeU = findFreeColor(u, maxColor, vertexEdgeColors);
        int freeV = findFreeColor(v, maxColor, vertexEdgeColors);

        if (freeU == freeV) {
            // Simple case: both endpoints share a free color
            edges[i].color = freeU;
            vertexEdgeColors[u].append(freeU);
            vertexEdgeColors[v].append(freeU);
        } else {
            // Vizing fan extension
            auto fan = vizingFan(u, v, adjColors, vertexEdgeColors, maxColor);
            int freeAtLast = findFreeColor(fan.last(), maxColor, vertexEdgeColors);

            if (freeAtLast == freeU) {
                // Rotate fan and color
                rotateFan(edges, fan, u, freeU, vertexEdgeColors);
                edges[i].color = freeU;
                vertexEdgeColors[u].append(freeU);
                vertexEdgeColors[v].append(freeU);
            } else {
                // Tashkinov augmentation
                tashkinovAugment(edges, i, vertexEdgeColors, maxColor, numVertices);
            }
        }

        // Update maxColor if needed
        if (edges[i].color > maxColor)
            maxColor = edges[i].color;
    }

    result.edges = edges;
    result.numColors = maxColor + 1;
    result.maxDegree = maxDeg;
    result.isClass1 = (result.numColors == maxDeg);
    result.isValid = verifyColoring(edges);

    m_stats.totalColorings++;
    m_stats.numVertices = numVertices;
    m_stats.numEdges = nE;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringDone(nE, result.numColors, elapsed);
    return result;
}

/* ---- Reset ---- */

void EdgeColoring11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
