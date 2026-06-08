/**
 * @file GraphColoring7.cpp
 * @brief GraphColoring7 实现
 *
 * 实现图着色：DSATUR启发式选择、前向检查回溯搜索。
 */

#include "utils/graph229/GraphColoring7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphColoring7::GraphColoring7(QObject *parent) : QObject(parent) {}
GraphColoring7::~GraphColoring7() = default;

/* ---- Set graph via adjacency matrix ---- */

void GraphColoring7::setGraph(const QVector<QVector<int>>& adjacency)
{
    m_adj = adjacency;
    m_n = adjacency.size();
    m_stats.numVertices = m_n;
    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (i < adjacency[i].size() && adjacency[i][j]) ++edges;
    m_stats.numEdges = edges;
}

/* ---- Set graph via edge list ---- */

void GraphColoring7::setGraphEdges(int numVertices,
                                    const QVector<QPair<int,int>>& edges)
{
    m_n = numVertices;
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_adj[i].resize(m_n, 0);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n &&
            e.second >= 0 && e.second < m_n) {
            m_adj[e.first][e.second] = 1;
            m_adj[e.second][e.first] = 1;
        }
    }
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Saturation degree ---- */

int GraphColoring7::saturationDegree(int vertex) const
{
    if (vertex < 0 || vertex >= m_n) return 0;
    QVector<bool> seen(m_n, false);
    // Count distinct colors used by neighbors (need current coloring)
    // For DSATUR heuristic without full coloring, count colored neighbors
    int sat = 0;
    for (int j = 0; j < m_n; ++j) {
        if (vertex < m_adj.size() && j < m_adj[vertex].size() &&
            m_adj[vertex][j] && !seen[j]) {
            seen[j] = true;
            ++sat;
        }
    }
    return sat;
}

/* ---- DSATUR vertex selection ---- */

int GraphColoring7::pickVertex(const QVector<int>& colors,
                                const QVector<QVector<bool>>& available) const
{
    int best = -1;
    int bestSat = -1;
    int bestDeg = -1;

    for (int v = 0; v < m_n; ++v) {
        if (colors[v] >= 0) continue;  // Already colored

        // Compute saturation: number of distinct neighbor colors
        QVector<bool> neighborColors(m_n, false);
        int sat = 0;
        for (int j = 0; j < m_n; ++j) {
            if (v < m_adj.size() && j < m_adj[v].size() &&
                m_adj[v][j] && colors[j] >= 0) {
                if (!neighborColors[colors[j]]) {
                    neighborColors[colors[j]] = true;
                    ++sat;
                }
            }
        }

        // Degree of uncolored neighbors
        int deg = 0;
        for (int j = 0; j < m_n; ++j) {
            if (v < m_adj.size() && j < m_adj[v].size() &&
                m_adj[v][j] && colors[j] < 0) ++deg;
        }

        if (sat > bestSat || (sat == bestSat && deg > bestDeg)) {
            bestSat = sat;
            bestDeg = deg;
            best = v;
        }
    }
    return best;
}

/* ---- Forward check ---- */

void GraphColoring7::forwardCheck(int vertex, int color,
                                   QVector<QVector<bool>>& available,
                                   QVector<int>& removed) const
{
    removed.clear();
    for (int j = 0; j < m_n; ++j) {
        if (vertex < m_adj.size() && j < m_adj[vertex].size() &&
            m_adj[vertex][j] && available[j][color]) {
            available[j][color] = false;
            removed.append(j);
        }
    }
}

/* ---- Undo forward check ---- */

void GraphColoring7::undoForwardCheck(int vertex,
                                       QVector<QVector<bool>>& available,
                                       const QVector<int>& removed) const
{
    Q_UNUSED(vertex)
    for (int j : removed)
        if (j >= 0 && j < available.size())
            for (int c = 0; c < available[j].size(); ++c)
                available[j][c] = true;
}

/* ---- Backtracking ---- */

bool GraphColoring7::backtrack(QVector<int>& colors,
                                QVector<QVector<bool>>& available,
                                int colored, int maxColors)
{
    if (colored == m_n) return true;

    int v = pickVertex(colors, available);
    if (v < 0) return false;

    for (int c = 0; c < maxColors; ++c) {
        if (!available[v][c]) continue;

        colors[v] = c;
        QVector<int> removed;
        // Simple forward check: remove color from neighbors
        for (int j = 0; j < m_n; ++j) {
            if (v < m_adj.size() && j < m_adj[v].size() &&
                m_adj[v][j] && available[j][c]) {
                available[j][c] = false;
                removed.append(j);
            }
        }

        // Check no neighbor has zero available colors
        bool deadEnd = false;
        for (int j : removed) {
            bool hasAvail = false;
            for (int cc = 0; cc < maxColors; ++cc) {
                if (available[j][cc]) { hasAvail = true; break; }
            }
            if (!hasAvail) { deadEnd = true; break; }
        }

        if (!deadEnd && backtrack(colors, available, colored + 1, maxColors))
            return true;

        // Undo
        m_stats.backtracks++;
        for (int j : removed) available[j][c] = true;
        colors[v] = -1;
    }
    return false;
}

/* ---- Greedy DSATUR ---- */

QVector<int> GraphColoring7::greedyDSATUR()
{
    QVector<int> colors(m_n, -1);
    QVector<QVector<bool>> avail(m_n, QVector<bool>(m_n, true));

    for (int step = 0; step < m_n; ++step) {
        int v = pickVertex(colors, avail);
        if (v < 0) break;

        // Pick smallest available color
        for (int c = 0; c < m_n; ++c) {
            if (!avail[v][c]) continue;
            colors[v] = c;
            for (int j = 0; j < m_n; ++j) {
                if (v < m_adj.size() && j < m_adj[v].size() && m_adj[v][j])
                    avail[j][c] = false;
            }
            break;
        }
    }
    return colors;
}

/* ---- Solve with backtracking ---- */

QVector<int> GraphColoring7::solve(int maxColors)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // Get upper bound from greedy
    QVector<int> greedy = greedyDSATUR();
    int upper = countColors(greedy);
    if (maxColors <= 0) maxColors = upper;

    QVector<int> bestColors = greedy;
    m_chromatic = upper;

    // Try to improve by reducing color count
    for (int k = maxColors; k >= 1; --k) {
        QVector<int> colors(m_n, -1);
        QVector<QVector<bool>> avail(m_n, QVector<bool>(k, true));
        m_stats.backtracks = 0;

        if (backtrack(colors, avail, 0, k)) {
            bestColors = colors;
            m_chromatic = k;
        } else {
            break;  // Can't do better
        }
    }

    m_bestColors = bestColors;
    m_stats.chromaticNumber = m_chromatic;
    m_stats.optimalFound = true;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringCompleted(m_chromatic, m_stats.backtracks, timer.elapsed());

    return bestColors;
}

/* ---- Verify coloring ---- */

bool GraphColoring7::isValidColoring(const QVector<int>& colors) const
{
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            if (i < m_adj.size() && j < m_adj[i].size() &&
                m_adj[i][j] && colors[i] == colors[j])
                return false;
        }
    }
    return true;
}

/* ---- Count colors ---- */

int GraphColoring7::countColors(const QVector<int>& colors) const
{
    int mx = -1;
    for (int c : colors) mx = qMax(mx, c);
    return mx + 1;
}

/* ---- Reset ---- */

void GraphColoring7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_bestColors.clear();
    m_n = 0;
    m_chromatic = 0;
}
