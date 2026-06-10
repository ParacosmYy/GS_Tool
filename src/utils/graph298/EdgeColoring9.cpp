/**
 * @file EdgeColoring9.cpp
 * @brief EdgeColoring9 实现
 *
 * 实现边着色：Tashma迭代增广与Vizing定理的Class I/II图分类。
 */

#include "utils/graph298/EdgeColoring9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EdgeColoring9::EdgeColoring9(QObject *parent)
    : QObject(parent) {}

EdgeColoring9::~EdgeColoring9() = default;

/* ---- Build adjacency ---- */

void EdgeColoring9::buildAdjacency(const QVector<QPair<int, int>>& edges, int n,
                                    QVector<QVector<int>>& adjEdges,
                                    QVector<QVector<int>>& adjVertices) const
{
    adjVertices.resize(n);
    adjEdges.resize(n);

    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].first;
        int v = edges[i].second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            adjEdges[u].append(i);
            adjEdges[v].append(i);
            adjVertices[u].append(v);
            adjVertices[v].append(u);
        }
    }
}

/* ---- Maximum degree ---- */

int EdgeColoring9::maxDegreeOf(const QVector<QVector<int>>& adjVertices) const
{
    int maxDeg = 0;
    for (const auto& adj : adjVertices)
        if (adj.size() > maxDeg) maxDeg = adj.size();
    return maxDeg;
}

/* ---- Missing color at vertex ---- */

int EdgeColoring9::missingColor(int vertex,
                                 const QVector<QVector<int>>& colorAt,
                                 int numColors) const
{
    for (int c = 0; c < numColors; ++c) {
        bool found = false;
        if (vertex < colorAt.size()) {
            for (int e : colorAt[vertex]) {
                if (e == c) { found = true; break; }
            }
        }
        if (!found) return c;
    }
    return -1;  // All colors used
}

/* ---- Kempe chain augmentation ---- */

bool EdgeColoring9::augmentKempe(int edgeIdx, QVector<Edge>& edges,
                                  QVector<QVector<int>>& colorAt,
                                  int numColors, int u, int v)
{
    int c_u = missingColor(u, colorAt, numColors);
    int c_v = missingColor(v, colorAt, numColors);

    if (c_u < 0 || c_v < 0) return false;
    if (c_u == c_v) {
        // Free color found directly
        return true;
    }

    // Build Kempe chain: alternate c_u, c_v from vertex v
    QVector<int> chain;
    int current = v;
    bool lookingFor = c_u;  // Start with c_u at v

    for (int step = 0; step < static_cast<int>(edges.size()) + 1; ++step) {
        int nextEdge = -1;
        int nextVertex = -1;

        // Find edge from 'current' with color 'lookingFor'
        for (int ei = 0; ei < edges.size(); ++ei) {
            if (edges[ei].color != lookingFor) continue;
            if (edges[ei].u == current) {
                nextVertex = edges[ei].v;
                nextEdge = ei;
                break;
            } else if (edges[ei].v == current) {
                nextVertex = edges[ei].u;
                nextEdge = ei;
                break;
            }
        }

        if (nextEdge < 0) break;  // No continuation
        chain.append(nextEdge);

        if (nextVertex == u) {
            // Kempe chain reached u: swap colors along chain
            for (int ci = 0; ci < chain.size(); ++ci) {
                int ei = chain[ci];
                edges[ei].color = (edges[ei].color == c_u) ? c_v : c_u;
            }
            return true;
        }

        current = nextVertex;
        lookingFor = (lookingFor == c_u) ? c_v : c_u;
    }

    // Chain didn't reach u: swap anyway (safe because chain is maximal)
    for (int ei : chain)
        edges[ei].color = (edges[ei].color == c_u) ? c_v : c_u;

    return true;
}

/* ---- Greedy coloring ---- */

void EdgeColoring9::greedyColor(QVector<Edge>& edges, int n, int numColors)
{
    // colorAt[vertex] = set of colors used at that vertex
    QVector<QVector<int>> colorAt(n);

    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].u;
        int v = edges[i].v;

        // Try direct color assignment
        int chosenColor = -1;
        for (int c = 0; c < numColors; ++c) {
            bool uHas = false, vHas = false;
            for (int ec : colorAt[u]) if (ec == c) { uHas = true; break; }
            for (int ec : colorAt[v]) if (ec == c) { vHas = true; break; }
            if (!uHas && !vHas) { chosenColor = c; break; }
        }

        if (chosenColor >= 0) {
            edges[i].color = chosenColor;
            colorAt[u].append(chosenColor);
            colorAt[v].append(chosenColor);
        } else {
            // Need augmentation
            if (augmentKempe(i, edges, colorAt, numColors, u, v)) {
                // Re-try assignment after augmentation
                for (int c = 0; c < numColors; ++c) {
                    bool uHas = false, vHas = false;
                    for (int ec : colorAt[u]) if (ec == c) { uHas = true; break; }
                    for (int ec : colorAt[v]) if (ec == c) { vHas = true; break; }
                    if (!uHas && !vHas) { chosenColor = c; break; }
                }
                if (chosenColor >= 0) {
                    edges[i].color = chosenColor;
                    colorAt[u].append(chosenColor);
                    colorAt[v].append(chosenColor);
                } else {
                    // Fallback: assign extra color (Class II)
                    edges[i].color = numColors;
                    colorAt[u].append(numColors);
                    colorAt[v].append(numColors);
                }
            }
        }
    }
}

/* ---- Validate coloring ---- */

bool EdgeColoring9::validateColoring(const QVector<Edge>& coloredEdges) const
{
    for (int i = 0; i < coloredEdges.size(); ++i) {
        for (int j = i + 1; j < coloredEdges.size(); ++j) {
            if (coloredEdges[i].color == coloredEdges[j].color) {
                // Check if edges share a vertex
                if (coloredEdges[i].u == coloredEdges[j].u ||
                    coloredEdges[i].u == coloredEdges[j].v ||
                    coloredEdges[i].v == coloredEdges[j].u ||
                    coloredEdges[i].v == coloredEdges[j].v)
                    return false;
            }
        }
    }
    return true;
}

/* ---- Class I check ---- */

bool EdgeColoring9::isClassI(int maxDegree, int numColors) const
{
    // Vizing's theorem: Delta <= chi' <= Delta + 1
    // Class I if chi' = Delta
    return numColors <= maxDegree;
}

/* ---- Main coloring ---- */

EdgeColoring9::ColoringResult EdgeColoring9::color(
    const QVector<QPair<int, int>>& edgeList, int numVertices)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    int n = numVertices;
    int m = edgeList.size();

    if (m == 0 || n <= 0) return result;

    QVector<QVector<int>> adjEdges, adjVertices;
    buildAdjacency(edgeList, n, adjEdges, adjVertices);

    int delta = maxDegreeOf(adjVertices);
    result.maxDegree = delta;

    // By Vizing: need at most Delta + 1 colors
    int numColors = delta + 1;

    // Build edge array
    result.edges.resize(m);
    for (int i = 0; i < m; ++i) {
        result.edges[i].u = edgeList[i].first;
        result.edges[i].v = edgeList[i].second;
        result.edges[i].color = -1;
    }

    greedyColor(result.edges, n, numColors);

    // Determine actual colors used
    int maxColor = 0;
    for (const auto& e : result.edges)
        if (e.color > maxColor) maxColor = e.color;

    result.numColors = maxColor + 1;
    result.isValid = validateColoring(result.edges);
    result.isClass1 = isClassI(delta, result.numColors);

    double elapsed = timer.elapsed();
    m_stats.numVertices = n;
    m_stats.numEdges = m;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringDone(m, result.numColors, result.isClass1, elapsed);

    return result;
}

/* ---- Reset ---- */

void EdgeColoring9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
