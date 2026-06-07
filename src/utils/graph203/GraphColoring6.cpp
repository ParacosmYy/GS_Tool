/**
 * @file GraphColoring6.cpp
 * @brief GraphColoring6 实现
 *
 * 实现图着色：DSATUR启发式排序、回溯搜索、色数下界(团数)、Welsh-Powell贪心。
 */

#include "utils/graph203/GraphColoring6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphColoring6::GraphColoring6(QObject *parent) : QObject(parent) {}
GraphColoring6::~GraphColoring6() = default;

/* ---- Configuration ---- */

void GraphColoring6::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_edgeCount = countEdges();
}

void GraphColoring6::setMaxColors(int max) { m_maxColors = qMax(0, max); }

/* ---- Count edges ---- */

int GraphColoring6::countEdges() const
{
    int edges = 0;
    for (int i = 0; i < m_adj.size(); ++i)
        edges += m_adj[i].size();
    return edges / 2; // Each edge counted twice
}

/* ---- DSATUR vertex selection ---- */

int GraphColoring6::pickDsaturVertex(const QVector<int>& colors,
                                     const QVector<int>& saturation) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < colors.size(); ++v) {
        if (colors[v] >= 0) continue; // Already colored
        if (saturation[v] > bestSat ||
            (saturation[v] == bestSat &&
             m_adj[v].size() > static_cast<size_t>(bestDeg))) {
            bestSat = saturation[v];
            bestDeg = static_cast<int>(m_adj[v].size());
            best = v;
        }
    }
    return best;
}

/* ---- Backtracking helper ---- */

bool GraphColoring6::backtrack(QVector<int>& colors, int idx, int maxC,
                               int& steps, const QVector<int>& order)
{
    if (idx >= order.size()) return true; // All colored

    int v = order[idx];
    steps++;

    // Try each color
    for (int c = 0; c < maxC; ++c) {
        // Check if color c conflicts with neighbors
        bool conflict = false;
        for (int nb : m_adj[v]) {
            if (colors[nb] == c) { conflict = true; break; }
        }
        if (conflict) continue;

        colors[v] = c;
        if (backtrack(colors, idx + 1, maxC, steps, order))
            return true;
        colors[v] = -1; // Undo
    }
    return false;
}

/* ---- Main color (DSATUR + backtracking) ---- */

QVector<int> GraphColoring6::color()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_adj.size();
    if (n == 0) return {};

    // Build DSATUR ordering
    QVector<int> colors(n, -1);
    QVector<int> saturation(n, 0);
    QVector<int> order;
    order.reserve(n);

    for (int step = 0; step < n; ++step) {
        int v = pickDsaturVertex(colors, saturation);
        if (v < 0) break;

        // Find smallest available color
        QVector<bool> used(n, false);
        for (int nb : m_adj[v])
            if (colors[nb] >= 0) used[colors[nb]] = true;

        int c = 0;
        while (c < n && used[c]) c++;
        colors[v] = c;
        order.append(v);

        // Update saturation degrees
        for (int nb : m_adj[v]) {
            if (colors[nb] >= 0) continue;
            QVector<bool> neighborColors(n, false);
            for (int nn : m_adj[nb])
                if (colors[nn] >= 0) neighborColors[colors[nn]] = true;
            saturation[nb] = 0;
            for (bool uc : neighborColors) if (uc) saturation[nb]++;
        }
    }

    m_colorsUsed = 0;
    for (int c : colors) m_colorsUsed = qMax(m_colorsUsed, c + 1);

    // Try backtracking to improve if maxColors is set
    int targetColors = m_maxColors > 0 ? qMin(m_maxColors, m_colorsUsed) : m_colorsUsed;
    if (targetColors < m_colorsUsed) {
        QVector<int> btColors(n, -1);
        int steps = 0;
        if (backtrack(btColors, 0, targetColors, steps, order)) {
            colors = btColors;
            m_colorsUsed = targetColors;
        }
        m_stats.backtrackSteps += steps;
    }

    m_stats.totalRuns++;
    m_stats.numVertices = n;
    m_stats.numEdges = m_edgeCount;
    m_stats.colorsUsed = m_colorsUsed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit coloringCompleted(n, m_colorsUsed, m_stats.backtrackSteps, timer.elapsed());
    return colors;
}

/* ---- Chromatic number lower bound (max clique size) ---- */

int GraphColoring6::maxCliqueSize() const
{
    int n = m_adj.size();
    if (n == 0) return 0;

    // Greedy clique: start from each vertex, greedily add common neighbors
    int maxSize = 1;
    for (int start = 0; start < n; ++start) {
        QVector<int> clique = {start};
        QVector<bool> candidates(n, false);
        for (int nb : m_adj[start]) candidates[nb] = true;

        for (int v = 0; v < n; ++v) {
            if (!candidates[v]) continue;
            bool allConnected = true;
            for (int c : clique) {
                bool connected = false;
                for (int nb : m_adj[c]) {
                    if (nb == v) { connected = true; break; }
                }
                if (!connected) { allConnected = false; break; }
            }
            if (allConnected) {
                clique.append(v);
                // Restrict candidates to neighbors of v
                for (int j = 0; j < n; ++j) {
                    if (!candidates[j] || j == v) continue;
                    bool isNb = false;
                    for (int nb : m_adj[v]) if (nb == j) { isNb = true; break; }
                    if (!isNb) candidates[j] = false;
                }
            }
        }
        maxSize = qMax(maxSize, static_cast<int>(clique.size()));
    }
    return maxSize;
}

int GraphColoring6::chromaticLowerBound() const { return maxCliqueSize(); }

/* ---- Verify coloring ---- */

bool GraphColoring6::verifyColoring(const QVector<int>& coloring) const
{
    for (int v = 0; v < m_adj.size(); ++v) {
        for (int nb : m_adj[v]) {
            if (v < nb && coloring[v] == coloring[nb]) return false;
        }
    }
    return true;
}

/* ---- Greedy Welsh-Powell coloring ---- */

QVector<int> GraphColoring6::greedyColor() const
{
    int n = m_adj.size();
    QVector<int> colors(n, -1);

    // Sort vertices by degree descending
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    for (int v : order) {
        QVector<bool> used(n, false);
        for (int nb : m_adj[v])
            if (colors[nb] >= 0) used[colors[nb]] = true;
        int c = 0;
        while (c < n && used[c]) c++;
        colors[v] = c;
    }
    return colors;
}

/* ---- Reset statistics ---- */

void GraphColoring6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_colorsUsed = 0;
}
