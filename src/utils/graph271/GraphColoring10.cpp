/**
 * @file GraphColoring10.cpp
 * @brief GraphColoring10 实现
 *
 * 实现图着色：递归最大优先与独立集提取增量着色。
 */

#include "utils/graph271/GraphColoring10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphColoring10::GraphColoring10(QObject *parent)
    : QObject(parent) {}
GraphColoring10::~GraphColoring10() = default;

/* ---- Graph manipulation ---- */

void GraphColoring10::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_numVertices = adjacency.size();
}

void GraphColoring10::addVertex(const QVector<int>& neighbors)
{
    int v = m_numVertices++;
    m_adj.append(neighbors);
    // Add bidirectional edges
    for (int u : neighbors) {
        if (u >= 0 && u < m_adj.size())
            m_adj[u].append(v);
    }
}

void GraphColoring10::removeVertex(int v)
{
    if (v < 0 || v >= m_numVertices) return;
    // Remove v from all neighbor lists
    for (int u : m_adj[v]) {
        if (u >= 0 && u < m_adj.size()) {
            m_adj[u].removeAll(v);
        }
    }
    m_adj[v].clear();
}

void GraphColoring10::addEdge(int u, int v)
{
    if (u < 0 || v < 0 || u >= m_numVertices || v >= m_numVertices) return;
    if (!m_adj[u].contains(v)) m_adj[u].append(v);
    if (!m_adj[v].contains(u)) m_adj[v].append(u);
}

/* ---- Find vertex with most uncolored neighbors ---- */

int GraphColoring10::findMaxDegree(const QVector<bool>& colored) const
{
    int best = -1, bestDeg = -1;
    for (int v = 0; v < m_numVertices; ++v) {
        if (colored[v]) continue;
        int deg = 0;
        for (int u : m_adj[v])
            if (!colored[u]) deg++;
        if (deg > bestDeg) { bestDeg = deg; best = v; }
    }
    return best;
}

/* ---- Find uncolored vertex with most neighbors in set ---- */

int GraphColoring10::findMaxAdjInSet(const QVector<bool>& colored,
                                      const QVector<bool>& inSet) const
{
    int best = -1, bestAdj = -1;
    for (int v = 0; v < m_numVertices; ++v) {
        if (colored[v]) continue;
        int adjCount = 0;
        for (int u : m_adj[v])
            if (inSet[u]) adjCount++;
        if (adjCount > bestAdj) { bestAdj = adjCount; best = v; }
    }
    return best;
}

/* ---- Extract one independent set via RLF ---- */

GraphColoring10::IndependentSet GraphColoring10::extractOneIS(
    QVector<bool>& colored, int colorIdx)
{
    IndependentSet is;
    is.color = colorIdx;
    QVector<bool> inSet(m_numVertices, false);

    // Start with vertex of maximum degree among uncolored
    int v = findMaxDegree(colored);
    if (v < 0) return is;

    // Candidates: uncolored vertices not adjacent to any in the IS
    QVector<bool> candidate(m_numVertices, true);
    for (int u : m_adj[v]) candidate[u] = false;

    while (v >= 0) {
        is.vertices.append(v);
        colored[v] = true;
        inSet[v] = true;

        // Remove neighbors of v from candidates
        for (int u : m_adj[v])
            candidate[u] = false;

        // Pick next candidate with most neighbors in current IS
        v = -1;
        int bestAdj = -1;
        for (int c = 0; c < m_numVertices; ++c) {
            if (!candidate[c] || colored[c]) continue;
            int adjCount = 0;
            for (int u : m_adj[c])
                if (inSet[u]) adjCount++;
            if (adjCount > bestAdj) { bestAdj = adjCount; v = c; }
        }
    }
    return is;
}

/* ---- Greedy sequential coloring ---- */

QVector<int> GraphColoring10::greedyColoring() const
{
    QVector<int> colors(m_numVertices, -1);
    for (int v = 0; v < m_numVertices; ++v) {
        QVector<bool> used(m_numVertices, false);
        for (int u : m_adj[v])
            if (colors[u] >= 0) used[colors[u]] = true;
        for (int c = 0; c < m_numVertices; ++c) {
            if (!used[c]) { colors[v] = c; break; }
        }
    }
    return colors;
}

/* ---- RLF coloring ---- */

GraphColoring10::ColoringResult GraphColoring10::colorRLF()
{
    QElapsedTimer timer;
    timer.start();

    QVector<bool> colored(m_numVertices, false);
    QVector<int> colors(m_numVertices, -1);
    int numColors = 0;

    while (true) {
        IndependentSet is = extractOneIS(colored, numColors);
        if (is.vertices.isEmpty()) break;
        for (int v : is.vertices)
            colors[v] = numColors;
        numColors++;
    }

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = 0;
    for (int v = 0; v < m_numVertices; ++v)
        m_stats.numEdges += m_adj[v].size();
    m_stats.numEdges /= 2;
    m_stats.numColors = numColors;
    m_stats.numIndependentSets = numColors;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    ColoringResult result;
    result.colors = colors;
    result.numColors = numColors;
    result.isValid = verifyColoring(colors);

    emit coloringCompleted(numColors, m_numVertices, elapsed);
    return result;
}

/* ---- Verify coloring ---- */

bool GraphColoring10::verifyColoring(const QVector<int>& colors) const
{
    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] < 0) return false;
        for (int u : m_adj[v])
            if (colors[v] == colors[u]) return false;
    }
    return true;
}

/* ---- Extract independent sets ---- */

QVector<GraphColoring10::IndependentSet> GraphColoring10::extractIndependentSets() const
{
    QVector<IndependentSet> sets;
    QVector<int> colors = greedyColoring();
    int maxColor = 0;
    for (int c : colors) maxColor = qMax(maxColor, c);

    for (int c = 0; c <= maxColor; ++c) {
        IndependentSet is;
        is.color = c;
        for (int v = 0; v < m_numVertices; ++v)
            if (colors[v] == c) is.vertices.append(v);
        sets.append(is);
    }
    return sets;
}

/* ---- Incremental recoloring ---- */

GraphColoring10::ColoringResult GraphColoring10::recolorIncremental(
    const QVector<int>& changedVertices)
{
    // Full recolor is the baseline incremental strategy
    Q_UNUSED(changedVertices)
    return colorRLF();
}

/* ---- Reset ---- */

void GraphColoring10::resetStatistics()
{
    m_adj.clear();
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
