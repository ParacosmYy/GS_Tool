/**
 * @file GraphColoring13.cpp
 * @brief GraphColoring13 实现
 *
 * 实现图着色：递归最大优先与饱和度排序实现紧界顺序顶点着色。
 */

#include "utils/graph314/GraphColoring13.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphColoring13::GraphColoring13(QObject *parent)
    : QObject(parent) {}

GraphColoring13::~GraphColoring13() = default;

/* ---- Build adjacency list ---- */

void GraphColoring13::setGraph(int numVertices, const QVector<QPair<int, int>>& edges)
{
    m_numVertices = numVertices;
    m_adj.clear();
    m_adj.resize(numVertices);

    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < numVertices &&
            edge.second >= 0 && edge.second < numVertices) {
            m_adj[edge.first].append(edge.second);
            m_adj[edge.second].append(edge.first);
        }
    }

    // Remove duplicate edges
    for (int i = 0; i < numVertices; ++i) {
        std::sort(m_adj[i].begin(), m_adj[i].end());
        m_adj[i].erase(std::unique(m_adj[i].begin(), m_adj[i].end()), m_adj[i].end());
    }
}

/* ---- Find lowest available color for vertex ---- */

int GraphColoring13::findLowestColor(int vertex, const QVector<int>& colors) const
{
    int n = m_numVertices;
    // Track which colors are used by neighbors
    QVector<bool> used(n + 1, false);
    for (int nb : m_adj[vertex]) {
        if (colors[nb] >= 0 && colors[nb] <= n)
            used[colors[nb]] = true;
    }
    // Greedy: find first unused color
    for (int c = 0; c <= n; ++c) {
        if (!used[c]) return c;
    }
    return n;  // fallback
}

/* ---- Select next DSATUR vertex ---- */

int GraphColoring13::selectDSATURVertex(const QVector<int>& colors,
                                          const QVector<int>& saturation) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] >= 0) continue;  // already colored
        int sat = saturation[v];
        // Count uncolored degree for tie-breaking
        int deg = 0;
        for (int nb : m_adj[v])
            if (colors[nb] < 0) ++deg;

        if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
            bestSat = sat;
            bestDeg = deg;
            best = v;
        }
    }
    return best;
}

/* ---- Update saturation degrees ---- */

void GraphColoring13::updateSaturation(int vertex, const QVector<int>& colors,
                                        QVector<int>& saturation) const
{
    // For each uncolored neighbor, recompute saturation
    for (int nb : m_adj[vertex]) {
        if (colors[nb] >= 0) continue;
        // Count distinct colors among nb's neighbors
        QVector<int> seenColors;
        for (int nnb : m_adj[nb]) {
            if (colors[nnb] >= 0 && !seenColors.contains(colors[nnb]))
                seenColors.append(colors[nnb]);
        }
        saturation[nb] = seenColors.size();
    }
}

/* ---- DSATUR coloring ---- */

GraphColoring13::ColoringResult GraphColoring13::colorDSATUR()
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    result.colors.resize(m_numVertices, -1);
    QVector<int> saturation(m_numVertices, 0);

    for (int step = 0; step < m_numVertices; ++step) {
        // Select vertex with max saturation, tie-break by degree
        int v = selectDSATURVertex(result.colors, saturation);
        if (v < 0) break;

        // Assign lowest possible color
        result.colors[v] = findLowestColor(v, result.colors);

        // Update saturation for uncolored neighbors
        updateSaturation(v, result.colors, saturation);
    }

    // Count colors used
    int maxColor = 0;
    for (int c : result.colors)
        maxColor = qMax(maxColor, c);
    result.numColors = maxColor + 1;
    result.isValid = verifyColoring(result.colors);

    // Compute density
    int totalEdges = 0;
    for (int i = 0; i < m_numVertices; ++i)
        totalEdges += m_adj[i].size();
    totalEdges /= 2;
    int maxEdges = m_numVertices * (m_numVertices - 1) / 2;
    result.density = (maxEdges > 0) ? static_cast<double>(totalEdges) / maxEdges : 0.0;

    m_stats.totalColorings++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = totalEdges;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringDone(m_numVertices, result.numColors, elapsed);
    return result;
}

/* ---- Find uncolored non-adjacent to current color class ---- */

QVector<int> GraphColoring13::findUncoloredNonAdjacent(const QVector<int>& colors,
                                                         int color,
                                                         const QVector<int>& candidates) const
{
    QVector<int> result;
    for (int v : candidates) {
        if (colors[v] >= 0) continue;
        // Check if v is adjacent to any vertex with 'color'
        bool adjacent = false;
        for (int nb : m_adj[v]) {
            if (colors[nb] == color) { adjacent = true; break; }
        }
        if (!adjacent) result.append(v);
    }
    return result;
}

/* ---- RLF (Recursive Largest First) coloring ---- */

GraphColoring13::ColoringResult GraphColoring13::colorRLF()
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    result.colors.resize(m_numVertices, -1);
    int currentColor = 0;

    // All vertices start as uncolored candidates
    QVector<int> uncolored;
    for (int i = 0; i < m_numVertices; ++i) uncolored.append(i);

    while (!uncolored.isEmpty()) {
        // Find vertex with max degree among uncolored
        int maxDeg = -1, seed = -1;
        for (int v : uncolored) {
            int deg = 0;
            for (int nb : m_adj[v])
                if (result.colors[nb] < 0) ++deg;
            if (deg > maxDeg) { maxDeg = deg; seed = v; }
        }

        if (seed < 0) break;

        // Color seed with current color
        result.colors[seed] = currentColor;

        // Build color class: add non-adjacent uncolored vertices
        QVector<int> candidates;
        for (int v : uncolored)
            if (v != seed && result.colors[v] < 0) candidates.append(v);

        while (!candidates.isEmpty()) {
            // Find candidate with max neighbors already in this color class
            // Among those not adjacent to any vertex of current color
            auto nonAdj = findUncoloredNonAdjacent(result.colors, currentColor, candidates);
            if (nonAdj.isEmpty()) break;

            int bestV = -1, bestCommon = -1;
            for (int v : nonAdj) {
                int common = 0;
                for (int nb : m_adj[v])
                    if (result.colors[nb] == currentColor) ++common;
                if (common > bestCommon) { bestCommon = common; bestV = v; }
            }

            if (bestV < 0) break;
            result.colors[bestV] = currentColor;

            // Remove colored vertex and its neighbors from candidates
            QVector<int> newCandidates;
            for (int v : candidates) {
                if (v == bestV) continue;
                bool adjToBest = false;
                for (int nb : m_adj[bestV]) {
                    if (nb == v) { adjToBest = true; break; }
                }
                if (!adjToBest) newCandidates.append(v);
            }
            candidates = newCandidates;
        }

        // Update uncolored list
        uncolored.clear();
        for (int i = 0; i < m_numVertices; ++i)
            if (result.colors[i] < 0) uncolored.append(i);

        currentColor++;
    }

    result.numColors = currentColor;
    result.isValid = verifyColoring(result.colors);

    int totalEdges = 0;
    for (int i = 0; i < m_numVertices; ++i)
        totalEdges += m_adj[i].size();
    totalEdges /= 2;
    int maxEdges = m_numVertices * (m_numVertices - 1) / 2;
    result.density = (maxEdges > 0) ? static_cast<double>(totalEdges) / maxEdges : 0.0;

    m_stats.totalColorings++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = totalEdges;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    emit coloringDone(m_numVertices, result.numColors, elapsed);
    return result;
}

/* ---- Verify coloring ---- */

bool GraphColoring13::verifyColoring(const QVector<int>& colors) const
{
    for (int v = 0; v < m_numVertices; ++v) {
        if (colors[v] < 0) continue;
        for (int nb : m_adj[v]) {
            if (colors[nb] >= 0 && colors[v] == colors[nb])
                return false;  // conflict
        }
    }
    return true;
}

/* ---- Reset ---- */

void GraphColoring13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_numVertices = 0;
}
